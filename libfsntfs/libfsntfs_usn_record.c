/*
 * USN record functions
 *
 * Copyright (C) 2010-2026, Joachim Metz <joachim.metz@gmail.com>
 *
 * Refer to AUTHORS for acknowledgements.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */

#include <common.h>
#include <byte_stream.h>
#include <memory.h>
#include <types.h>

#include "libfsntfs_attribute.h"
#include "libfsntfs_definitions.h"
#include "libfsntfs_file_entry.h"
#include "libfsntfs_libcerror.h"
#include "libfsntfs_libuna.h"
#include "libfsntfs_usn_record.h"
#include "libfsntfs_volume.h"

/* The USN_RECORD_V2 header size
 */
#define LIBFSNTFS_USN_RECORD_V2_HEADER_SIZE	60

/* Retrieves the index of the first $FILE_NAME attribute of a file entry
 * Returns 1 if successful, 0 if not available or -1 on error
 */
static int libfsntfs_internal_usn_record_get_file_name_attribute_index(
            libfsntfs_file_entry_t *file_entry,
            int *attribute_index,
            libcerror_error_t **error )
{
	libfsntfs_attribute_t *attribute = NULL;
	static char *function            = "libfsntfs_internal_usn_record_get_file_name_attribute_index";
	uint32_t attribute_type          = 0;
	int number_of_attributes         = 0;
	int safe_attribute_index         = 0;

	if( libfsntfs_file_entry_get_number_of_attributes(
	     file_entry,
	     &number_of_attributes,
	     error ) != 1 )
	{
		libcerror_error_set(
		 error,
		 LIBCERROR_ERROR_DOMAIN_RUNTIME,
		 LIBCERROR_RUNTIME_ERROR_GET_FAILED,
		 "%s: unable to retrieve number of attributes.",
		 function );

		return( -1 );
	}
	for( safe_attribute_index = 0;
	     safe_attribute_index < number_of_attributes;
	     safe_attribute_index++ )
	{
		attribute = NULL;

		if( libfsntfs_file_entry_get_attribute_by_index(
		     file_entry,
		     safe_attribute_index,
		     &attribute,
		     error ) != 1 )
		{
			libcerror_error_set(
			 error,
			 LIBCERROR_ERROR_DOMAIN_RUNTIME,
			 LIBCERROR_RUNTIME_ERROR_GET_FAILED,
			 "%s: unable to retrieve attribute: %d.",
			 function,
			 safe_attribute_index );

			return( -1 );
		}
		if( libfsntfs_attribute_get_type(
		     attribute,
		     &attribute_type,
		     error ) != 1 )
		{
			libcerror_error_set(
			 error,
			 LIBCERROR_ERROR_DOMAIN_RUNTIME,
			 LIBCERROR_RUNTIME_ERROR_GET_FAILED,
			 "%s: unable to retrieve attribute: %d type.",
			 function,
			 safe_attribute_index );

			return( -1 );
		}
		if( attribute_type == LIBFSNTFS_ATTRIBUTE_TYPE_FILE_NAME )
		{
			*attribute_index = safe_attribute_index;

			return( 1 );
		}
	}
	return( 0 );
}

/* Retrieves a file entry that matches a file reference
 * Returns 1 if successful, 0 if no matching file entry or -1 on error
 */
static int libfsntfs_internal_usn_record_get_file_entry_by_reference(
            uint64_t file_reference,
            libfsntfs_volume_t *volume,
            libfsntfs_file_entry_t **file_entry,
            libcerror_error_t **error )
{
	libfsntfs_file_entry_t *safe_file_entry = NULL;
	static char *function                   = "libfsntfs_internal_usn_record_get_file_entry_by_reference";
	uint64_t matching_file_reference        = 0;
	uint64_t mft_entry_index                = 0;
	int result                              = 0;

	mft_entry_index = file_reference & 0x0000ffffffffffffUL;

	result = libfsntfs_volume_get_file_entry_by_index(
	          volume,
	          mft_entry_index,
	          &safe_file_entry,
	          error );

	if( result == -1 )
	{
		libcerror_error_set(
		 error,
		 LIBCERROR_ERROR_DOMAIN_RUNTIME,
		 LIBCERROR_RUNTIME_ERROR_GET_FAILED,
		 "%s: unable to retrieve file entry: %" PRIu64 ".",
		 function,
		 mft_entry_index );

		return( -1 );
	}
	if( result == 0 )
	{
		return( 0 );
	}
	if( libfsntfs_file_entry_get_file_reference(
	     safe_file_entry,
	     &matching_file_reference,
	     error ) != 1 )
	{
		libcerror_error_set(
		 error,
		 LIBCERROR_ERROR_DOMAIN_RUNTIME,
		 LIBCERROR_RUNTIME_ERROR_GET_FAILED,
		 "%s: unable to retrieve file reference.",
		 function );

		goto on_error;
	}
	/* A reused MFT entry is a different file. Do not associate a stale
	 * USN record with the new file.
	 */
	if( matching_file_reference != file_reference )
	{
		if( libfsntfs_file_entry_free(
		     &safe_file_entry,
		     error ) != 1 )
		{
			libcerror_error_set(
			 error,
			 LIBCERROR_ERROR_DOMAIN_RUNTIME,
			 LIBCERROR_RUNTIME_ERROR_FINALIZE_FAILED,
			 "%s: unable to free file entry.",
			 function );

			return( -1 );
		}
		return( 0 );
	}
	*file_entry = safe_file_entry;

	return( 1 );

on_error:
	if( safe_file_entry != NULL )
	{
		libfsntfs_file_entry_free(
		 &safe_file_entry,
		 NULL );
	}
	return( -1 );
}

/* Retrieves the parent path size of an USN record
 * The returned size includes the end of string character
 * Returns 1 if successful, 0 if no matching parent path or -1 on error
 */
static int libfsntfs_internal_usn_record_get_parent_path_size(
            libfsntfs_internal_usn_record_t *internal_usn_record,
            libfsntfs_volume_t *volume,
            size_t *parent_path_size,
            libcerror_error_t **error )
{
	libfsntfs_file_entry_t *parent_file_entry = NULL;
	static char *function                     = "libfsntfs_internal_usn_record_get_parent_path_size";
	uint64_t parent_mft_entry_index           = 0;
	int attribute_index                       = 0;
	int result                                = 0;

	parent_mft_entry_index = internal_usn_record->parent_file_reference & 0x0000ffffffffffffUL;

	if( parent_mft_entry_index == LIBFSNTFS_MFT_ENTRY_INDEX_ROOT_DIRECTORY )
	{
		*parent_path_size = 2;

		return( 1 );
	}
	result = libfsntfs_internal_usn_record_get_file_entry_by_reference(
	          internal_usn_record->parent_file_reference,
	          volume,
	          &parent_file_entry,
	          error );

	if( result == -1 )
	{
		libcerror_error_set(
		 error,
		 LIBCERROR_ERROR_DOMAIN_RUNTIME,
		 LIBCERROR_RUNTIME_ERROR_GET_FAILED,
		 "%s: unable to retrieve parent file entry.",
		 function );

		return( -1 );
	}
	if( result == 0 )
	{
		return( 0 );
	}
	result = libfsntfs_internal_usn_record_get_file_name_attribute_index(
	          parent_file_entry,
	          &attribute_index,
	          error );

	if( result == -1 )
	{
		libcerror_error_set(
		 error,
		 LIBCERROR_ERROR_DOMAIN_RUNTIME,
		 LIBCERROR_RUNTIME_ERROR_GET_FAILED,
		 "%s: unable to retrieve parent name attribute index.",
		 function );

		goto on_error;
	}
	if( result == 0 )
	{
		libfsntfs_file_entry_free(
		 &parent_file_entry,
		 NULL );

		return( 0 );
	}
	result = libfsntfs_file_entry_get_utf8_path_hint_size(
	          parent_file_entry,
	          attribute_index,
	          parent_path_size,
	          error );

	if( result != 1 )
	{
		libcerror_error_set(
		 error,
		 LIBCERROR_ERROR_DOMAIN_RUNTIME,
		 LIBCERROR_RUNTIME_ERROR_GET_FAILED,
		 "%s: unable to retrieve parent path size.",
		 function );

		result = -1;
	}
	if( libfsntfs_file_entry_free(
	     &parent_file_entry,
	     error ) != 1 )
	{
		libcerror_error_set(
		 error,
		 LIBCERROR_ERROR_DOMAIN_RUNTIME,
		 LIBCERROR_RUNTIME_ERROR_FINALIZE_FAILED,
		 "%s: unable to free parent file entry.",
		 function );

		return( -1 );
	}
	return( result );

on_error:
	if( parent_file_entry != NULL )
	{
		libfsntfs_file_entry_free(
		 &parent_file_entry,
		 NULL );
	}
	return( -1 );
}

/* Creates an USN record from a byte stream
 * Make sure the value usn_record is referencing, is set to NULL
 * Returns 1 if successful or -1 on error
 */
int libfsntfs_usn_record_initialize(
     libfsntfs_usn_record_t **usn_record,
     const uint8_t *data,
     size_t data_size,
     libcerror_error_t **error )
{
	libfsntfs_internal_usn_record_t *internal_usn_record = NULL;
	static char *function                                = "libfsntfs_usn_record_initialize";
	size_t name_offset                                   = 0;
	size_t name_size                                     = 0;
	uint32_t record_size                                 = 0;
	uint16_t major_version                               = 0;
	uint16_t minor_version                               = 0;

	if( usn_record == NULL )
	{
		libcerror_error_set(
		 error,
		 LIBCERROR_ERROR_DOMAIN_ARGUMENTS,
		 LIBCERROR_ARGUMENT_ERROR_INVALID_VALUE,
		 "%s: invalid USN record.",
		 function );

		return( -1 );
	}
	if( *usn_record != NULL )
	{
		libcerror_error_set(
		 error,
		 LIBCERROR_ERROR_DOMAIN_RUNTIME,
		 LIBCERROR_RUNTIME_ERROR_VALUE_ALREADY_SET,
		 "%s: invalid USN record value already set.",
		 function );

		return( -1 );
	}
	if( data == NULL )
	{
		libcerror_error_set(
		 error,
		 LIBCERROR_ERROR_DOMAIN_ARGUMENTS,
		 LIBCERROR_ARGUMENT_ERROR_INVALID_VALUE,
		 "%s: invalid data.",
		 function );

		return( -1 );
	}
	if( ( data_size < LIBFSNTFS_USN_RECORD_V2_HEADER_SIZE )
	 || ( data_size > (size_t) SSIZE_MAX ) )
	{
		libcerror_error_set(
		 error,
		 LIBCERROR_ERROR_DOMAIN_ARGUMENTS,
		 LIBCERROR_ARGUMENT_ERROR_VALUE_OUT_OF_BOUNDS,
		 "%s: invalid data size value out of bounds.",
		 function );

		return( -1 );
	}
	byte_stream_copy_to_uint32_little_endian(
	 data,
	 record_size );

	byte_stream_copy_to_uint16_little_endian(
	 &( data[ 4 ] ),
	 major_version );

	byte_stream_copy_to_uint16_little_endian(
	 &( data[ 6 ] ),
	 minor_version );

	if( ( record_size < LIBFSNTFS_USN_RECORD_V2_HEADER_SIZE )
	 || ( record_size > data_size ) )
	{
		libcerror_error_set(
		 error,
		 LIBCERROR_ERROR_DOMAIN_RUNTIME,
		 LIBCERROR_RUNTIME_ERROR_VALUE_OUT_OF_BOUNDS,
		 "%s: invalid record size value out of bounds.",
		 function );

		return( -1 );
	}
	if( major_version != 2 )
	{
		libcerror_error_set(
		 error,
		 LIBCERROR_ERROR_DOMAIN_ARGUMENTS,
		 LIBCERROR_ARGUMENT_ERROR_UNSUPPORTED_VALUE,
		 "%s: unsupported USN record version: %" PRIu16 ".%" PRIu16 ".",
		 function,
		 major_version,
		 minor_version );

		return( -1 );
	}
	byte_stream_copy_to_uint16_little_endian(
	 &( data[ 56 ] ),
	 name_size );

	byte_stream_copy_to_uint16_little_endian(
	 &( data[ 58 ] ),
	 name_offset );

	if( ( name_offset < LIBFSNTFS_USN_RECORD_V2_HEADER_SIZE )
	 || ( name_offset > record_size )
	 || ( name_size > ( record_size - name_offset ) )
	 || ( ( name_size & 1 ) != 0 ) )
	{
		libcerror_error_set(
		 error,
		 LIBCERROR_ERROR_DOMAIN_RUNTIME,
		 LIBCERROR_RUNTIME_ERROR_VALUE_OUT_OF_BOUNDS,
		 "%s: invalid name offset or size value out of bounds.",
		 function );

		return( -1 );
	}
	internal_usn_record = memory_allocate_structure(
	                       libfsntfs_internal_usn_record_t );

	if( internal_usn_record == NULL )
	{
		libcerror_error_set(
		 error,
		 LIBCERROR_ERROR_DOMAIN_MEMORY,
		 LIBCERROR_MEMORY_ERROR_INSUFFICIENT,
		 "%s: unable to create USN record.",
		 function );

		goto on_error;
	}
	if( memory_set(
	     internal_usn_record,
	     0,
	     sizeof( libfsntfs_internal_usn_record_t ) ) == NULL )
	{
		libcerror_error_set(
		 error,
		 LIBCERROR_ERROR_DOMAIN_MEMORY,
		 LIBCERROR_MEMORY_ERROR_SET_FAILED,
		 "%s: unable to clear USN record.",
		 function );

		goto on_error;
	}
	internal_usn_record->record_size = record_size;
	internal_usn_record->major_version = major_version;
	internal_usn_record->minor_version = minor_version;

	byte_stream_copy_to_uint64_little_endian(
	 &( data[ 8 ] ),
	 internal_usn_record->file_reference );

	byte_stream_copy_to_uint64_little_endian(
	 &( data[ 16 ] ),
	 internal_usn_record->parent_file_reference );

	byte_stream_copy_to_uint64_little_endian(
	 &( data[ 24 ] ),
	 internal_usn_record->update_sequence_number );

	byte_stream_copy_to_uint64_little_endian(
	 &( data[ 32 ] ),
	 internal_usn_record->update_time );

	byte_stream_copy_to_uint32_little_endian(
	 &( data[ 40 ] ),
	 internal_usn_record->update_reason_flags );

	byte_stream_copy_to_uint32_little_endian(
	 &( data[ 44 ] ),
	 internal_usn_record->update_source_flags );

	byte_stream_copy_to_uint32_little_endian(
	 &( data[ 48 ] ),
	 internal_usn_record->security_descriptor_identifier );

	byte_stream_copy_to_uint32_little_endian(
	 &( data[ 52 ] ),
	 internal_usn_record->file_attribute_flags );

	if( name_size > 0 )
	{
		internal_usn_record->name = (uint8_t *) memory_allocate(
		                                          sizeof( uint8_t ) * name_size );

		if( internal_usn_record->name == NULL )
		{
			libcerror_error_set(
			 error,
			 LIBCERROR_ERROR_DOMAIN_MEMORY,
			 LIBCERROR_MEMORY_ERROR_INSUFFICIENT,
			 "%s: unable to create name.",
			 function );

			goto on_error;
		}
		if( memory_copy(
		     internal_usn_record->name,
		     &( data[ name_offset ] ),
		     name_size ) == NULL )
		{
			libcerror_error_set(
			 error,
			 LIBCERROR_ERROR_DOMAIN_MEMORY,
			 LIBCERROR_MEMORY_ERROR_COPY_FAILED,
			 "%s: unable to copy name.",
			 function );

			goto on_error;
		}
		internal_usn_record->name_size = name_size;
	}
	*usn_record = (libfsntfs_usn_record_t *) internal_usn_record;

	return( 1 );

on_error:
	if( internal_usn_record != NULL )
	{
		if( internal_usn_record->name != NULL )
		{
			memory_free(
			 internal_usn_record->name );
		}
		memory_free(
		 internal_usn_record );
	}
	return( -1 );
}

/* Frees an USN record
 * Returns 1 if successful or -1 on error
 */
int libfsntfs_usn_record_free(
     libfsntfs_usn_record_t **usn_record,
     libcerror_error_t **error )
{
	libfsntfs_internal_usn_record_t *internal_usn_record = NULL;
	static char *function                                = "libfsntfs_usn_record_free";

	if( usn_record == NULL )
	{
		libcerror_error_set(
		 error,
		 LIBCERROR_ERROR_DOMAIN_ARGUMENTS,
		 LIBCERROR_ARGUMENT_ERROR_INVALID_VALUE,
		 "%s: invalid USN record.",
		 function );

		return( -1 );
	}
	if( *usn_record != NULL )
	{
		internal_usn_record = (libfsntfs_internal_usn_record_t *) *usn_record;
		*usn_record         = NULL;

		if( internal_usn_record->name != NULL )
		{
			memory_free(
			 internal_usn_record->name );
		}
		memory_free(
		 internal_usn_record );
	}
	return( 1 );
}

/* Retrieves the record size
 * Returns 1 if successful or -1 on error
 */
int libfsntfs_usn_record_get_size(
     libfsntfs_usn_record_t *usn_record,
     uint32_t *record_size,
     libcerror_error_t **error )
{
	libfsntfs_internal_usn_record_t *internal_usn_record = NULL;
	static char *function                                = "libfsntfs_usn_record_get_size";

	if( usn_record == NULL )
	{
		libcerror_error_set(
		 error,
		 LIBCERROR_ERROR_DOMAIN_ARGUMENTS,
		 LIBCERROR_ARGUMENT_ERROR_INVALID_VALUE,
		 "%s: invalid USN record.",
		 function );

		return( -1 );
	}
	internal_usn_record = (libfsntfs_internal_usn_record_t *) usn_record;

	if( record_size == NULL )
	{
		libcerror_error_set(
		 error,
		 LIBCERROR_ERROR_DOMAIN_ARGUMENTS,
		 LIBCERROR_ARGUMENT_ERROR_INVALID_VALUE,
		 "%s: invalid record size.",
		 function );

		return( -1 );
	}
	*record_size = internal_usn_record->record_size;

	return( 1 );
}

/* Retrieves the version
 * Returns 1 if successful or -1 on error
 */
int libfsntfs_usn_record_get_version(
     libfsntfs_usn_record_t *usn_record,
     uint16_t *major_version,
     uint16_t *minor_version,
     libcerror_error_t **error )
{
	libfsntfs_internal_usn_record_t *internal_usn_record = NULL;
	static char *function                                = "libfsntfs_usn_record_get_version";

	if( usn_record == NULL )
	{
		libcerror_error_set(
		 error,
		 LIBCERROR_ERROR_DOMAIN_ARGUMENTS,
		 LIBCERROR_ARGUMENT_ERROR_INVALID_VALUE,
		 "%s: invalid USN record.",
		 function );

		return( -1 );
	}
	internal_usn_record = (libfsntfs_internal_usn_record_t *) usn_record;

	if( major_version == NULL )
	{
		libcerror_error_set(
		 error,
		 LIBCERROR_ERROR_DOMAIN_ARGUMENTS,
		 LIBCERROR_ARGUMENT_ERROR_INVALID_VALUE,
		 "%s: invalid major version.",
		 function );

		return( -1 );
	}
	if( minor_version == NULL )
	{
		libcerror_error_set(
		 error,
		 LIBCERROR_ERROR_DOMAIN_ARGUMENTS,
		 LIBCERROR_ARGUMENT_ERROR_INVALID_VALUE,
		 "%s: invalid minor version.",
		 function );

		return( -1 );
	}
	*major_version = internal_usn_record->major_version;
	*minor_version = internal_usn_record->minor_version;

	return( 1 );
}

/* Retrieves the file reference
 * Returns 1 if successful or -1 on error
 */
int libfsntfs_usn_record_get_file_reference(
     libfsntfs_usn_record_t *usn_record,
     uint64_t *file_reference,
     libcerror_error_t **error )
{
	libfsntfs_internal_usn_record_t *internal_usn_record = NULL;
	static char *function                                = "libfsntfs_usn_record_get_file_reference";

	if( usn_record == NULL )
	{
		libcerror_error_set(
		 error,
		 LIBCERROR_ERROR_DOMAIN_ARGUMENTS,
		 LIBCERROR_ARGUMENT_ERROR_INVALID_VALUE,
		 "%s: invalid USN record.",
		 function );

		return( -1 );
	}
	internal_usn_record = (libfsntfs_internal_usn_record_t *) usn_record;

	if( file_reference == NULL )
	{
		libcerror_error_set(
		 error,
		 LIBCERROR_ERROR_DOMAIN_ARGUMENTS,
		 LIBCERROR_ARGUMENT_ERROR_INVALID_VALUE,
		 "%s: invalid file reference.",
		 function );

		return( -1 );
	}
	*file_reference = internal_usn_record->file_reference;

	return( 1 );
}

/* Retrieves the parent file reference
 * Returns 1 if successful or -1 on error
 */
int libfsntfs_usn_record_get_parent_file_reference(
     libfsntfs_usn_record_t *usn_record,
     uint64_t *parent_file_reference,
     libcerror_error_t **error )
{
	libfsntfs_internal_usn_record_t *internal_usn_record = NULL;
	static char *function                                = "libfsntfs_usn_record_get_parent_file_reference";

	if( usn_record == NULL )
	{
		libcerror_error_set(
		 error,
		 LIBCERROR_ERROR_DOMAIN_ARGUMENTS,
		 LIBCERROR_ARGUMENT_ERROR_INVALID_VALUE,
		 "%s: invalid USN record.",
		 function );

		return( -1 );
	}
	internal_usn_record = (libfsntfs_internal_usn_record_t *) usn_record;

	if( parent_file_reference == NULL )
	{
		libcerror_error_set(
		 error,
		 LIBCERROR_ERROR_DOMAIN_ARGUMENTS,
		 LIBCERROR_ARGUMENT_ERROR_INVALID_VALUE,
		 "%s: invalid parent file reference.",
		 function );

		return( -1 );
	}
	*parent_file_reference = internal_usn_record->parent_file_reference;

	return( 1 );
}

/* Retrieves the update sequence number
 * Returns 1 if successful or -1 on error
 */
int libfsntfs_usn_record_get_update_sequence_number(
     libfsntfs_usn_record_t *usn_record,
     uint64_t *update_sequence_number,
     libcerror_error_t **error )
{
	libfsntfs_internal_usn_record_t *internal_usn_record = NULL;
	static char *function                                = "libfsntfs_usn_record_get_update_sequence_number";

	if( usn_record == NULL )
	{
		libcerror_error_set(
		 error,
		 LIBCERROR_ERROR_DOMAIN_ARGUMENTS,
		 LIBCERROR_ARGUMENT_ERROR_INVALID_VALUE,
		 "%s: invalid USN record.",
		 function );

		return( -1 );
	}
	internal_usn_record = (libfsntfs_internal_usn_record_t *) usn_record;

	if( update_sequence_number == NULL )
	{
		libcerror_error_set(
		 error,
		 LIBCERROR_ERROR_DOMAIN_ARGUMENTS,
		 LIBCERROR_ARGUMENT_ERROR_INVALID_VALUE,
		 "%s: invalid update sequence number.",
		 function );

		return( -1 );
	}
	*update_sequence_number = internal_usn_record->update_sequence_number;

	return( 1 );
}

/* Retrieves the update time
 * Returns 1 if successful or -1 on error
 */
int libfsntfs_usn_record_get_update_time(
     libfsntfs_usn_record_t *usn_record,
     uint64_t *filetime,
     libcerror_error_t **error )
{
	libfsntfs_internal_usn_record_t *internal_usn_record = NULL;
	static char *function                                = "libfsntfs_usn_record_get_update_time";

	if( usn_record == NULL )
	{
		libcerror_error_set(
		 error,
		 LIBCERROR_ERROR_DOMAIN_ARGUMENTS,
		 LIBCERROR_ARGUMENT_ERROR_INVALID_VALUE,
		 "%s: invalid USN record.",
		 function );

		return( -1 );
	}
	internal_usn_record = (libfsntfs_internal_usn_record_t *) usn_record;

	if( filetime == NULL )
	{
		libcerror_error_set(
		 error,
		 LIBCERROR_ERROR_DOMAIN_ARGUMENTS,
		 LIBCERROR_ARGUMENT_ERROR_INVALID_VALUE,
		 "%s: invalid FILETIME.",
		 function );

		return( -1 );
	}
	*filetime = internal_usn_record->update_time;

	return( 1 );
}

/* Retrieves the update reason flags
 * Returns 1 if successful or -1 on error
 */
int libfsntfs_usn_record_get_update_reason_flags(
     libfsntfs_usn_record_t *usn_record,
     uint32_t *update_reason_flags,
     libcerror_error_t **error )
{
	libfsntfs_internal_usn_record_t *internal_usn_record = NULL;
	static char *function                                = "libfsntfs_usn_record_get_update_reason_flags";

	if( usn_record == NULL )
	{
		libcerror_error_set(
		 error,
		 LIBCERROR_ERROR_DOMAIN_ARGUMENTS,
		 LIBCERROR_ARGUMENT_ERROR_INVALID_VALUE,
		 "%s: invalid USN record.",
		 function );

		return( -1 );
	}
	internal_usn_record = (libfsntfs_internal_usn_record_t *) usn_record;

	if( update_reason_flags == NULL )
	{
		libcerror_error_set(
		 error,
		 LIBCERROR_ERROR_DOMAIN_ARGUMENTS,
		 LIBCERROR_ARGUMENT_ERROR_INVALID_VALUE,
		 "%s: invalid update reason flags.",
		 function );

		return( -1 );
	}
	*update_reason_flags = internal_usn_record->update_reason_flags;

	return( 1 );
}

/* Retrieves the update source flags
 * Returns 1 if successful or -1 on error
 */
int libfsntfs_usn_record_get_update_source_flags(
     libfsntfs_usn_record_t *usn_record,
     uint32_t *update_source_flags,
     libcerror_error_t **error )
{
	libfsntfs_internal_usn_record_t *internal_usn_record = NULL;
	static char *function                                = "libfsntfs_usn_record_get_update_source_flags";

	if( usn_record == NULL )
	{
		libcerror_error_set(
		 error,
		 LIBCERROR_ERROR_DOMAIN_ARGUMENTS,
		 LIBCERROR_ARGUMENT_ERROR_INVALID_VALUE,
		 "%s: invalid USN record.",
		 function );

		return( -1 );
	}
	internal_usn_record = (libfsntfs_internal_usn_record_t *) usn_record;

	if( update_source_flags == NULL )
	{
		libcerror_error_set(
		 error,
		 LIBCERROR_ERROR_DOMAIN_ARGUMENTS,
		 LIBCERROR_ARGUMENT_ERROR_INVALID_VALUE,
		 "%s: invalid update source flags.",
		 function );

		return( -1 );
	}
	*update_source_flags = internal_usn_record->update_source_flags;

	return( 1 );
}

/* Retrieves the security descriptor identifier
 * Returns 1 if successful or -1 on error
 */
int libfsntfs_usn_record_get_security_descriptor_identifier(
     libfsntfs_usn_record_t *usn_record,
     uint32_t *security_descriptor_identifier,
     libcerror_error_t **error )
{
	libfsntfs_internal_usn_record_t *internal_usn_record = NULL;
	static char *function                                = "libfsntfs_usn_record_get_security_descriptor_identifier";

	if( usn_record == NULL )
	{
		libcerror_error_set(
		 error,
		 LIBCERROR_ERROR_DOMAIN_ARGUMENTS,
		 LIBCERROR_ARGUMENT_ERROR_INVALID_VALUE,
		 "%s: invalid USN record.",
		 function );

		return( -1 );
	}
	internal_usn_record = (libfsntfs_internal_usn_record_t *) usn_record;

	if( security_descriptor_identifier == NULL )
	{
		libcerror_error_set(
		 error,
		 LIBCERROR_ERROR_DOMAIN_ARGUMENTS,
		 LIBCERROR_ARGUMENT_ERROR_INVALID_VALUE,
		 "%s: invalid security descriptor identifier.",
		 function );

		return( -1 );
	}
	*security_descriptor_identifier = internal_usn_record->security_descriptor_identifier;

	return( 1 );
}

/* Retrieves the file attribute flags
 * Returns 1 if successful or -1 on error
 */
int libfsntfs_usn_record_get_file_attribute_flags(
     libfsntfs_usn_record_t *usn_record,
     uint32_t *file_attribute_flags,
     libcerror_error_t **error )
{
	libfsntfs_internal_usn_record_t *internal_usn_record = NULL;
	static char *function                                = "libfsntfs_usn_record_get_file_attribute_flags";

	if( usn_record == NULL )
	{
		libcerror_error_set(
		 error,
		 LIBCERROR_ERROR_DOMAIN_ARGUMENTS,
		 LIBCERROR_ARGUMENT_ERROR_INVALID_VALUE,
		 "%s: invalid USN record.",
		 function );

		return( -1 );
	}
	internal_usn_record = (libfsntfs_internal_usn_record_t *) usn_record;

	if( file_attribute_flags == NULL )
	{
		libcerror_error_set(
		 error,
		 LIBCERROR_ERROR_DOMAIN_ARGUMENTS,
		 LIBCERROR_ARGUMENT_ERROR_INVALID_VALUE,
		 "%s: invalid file attribute flags.",
		 function );

		return( -1 );
	}
	*file_attribute_flags = internal_usn_record->file_attribute_flags;

	return( 1 );
}

/* Retrieves the size of the UTF-8 encoded name
 * The returned size includes the end of string character
 * Returns 1 if successful or -1 on error
 */
int libfsntfs_usn_record_get_utf8_name_size(
     libfsntfs_usn_record_t *usn_record,
     size_t *utf8_string_size,
     libcerror_error_t **error )
{
	libfsntfs_internal_usn_record_t *internal_usn_record = NULL;
	static char *function                                = "libfsntfs_usn_record_get_utf8_name_size";

	if( usn_record == NULL )
	{
		libcerror_error_set(
		 error,
		 LIBCERROR_ERROR_DOMAIN_ARGUMENTS,
		 LIBCERROR_ARGUMENT_ERROR_INVALID_VALUE,
		 "%s: invalid USN record.",
		 function );

		return( -1 );
	}
	internal_usn_record = (libfsntfs_internal_usn_record_t *) usn_record;

	if( libuna_utf8_string_size_from_utf16_stream(
	     internal_usn_record->name,
	     internal_usn_record->name_size,
	     LIBUNA_ENDIAN_LITTLE | LIBUNA_UTF16_STREAM_ALLOW_UNPAIRED_SURROGATE,
	     utf8_string_size,
	     error ) != 1 )
	{
		libcerror_error_set(
		 error,
		 LIBCERROR_ERROR_DOMAIN_RUNTIME,
		 LIBCERROR_RUNTIME_ERROR_GET_FAILED,
		 "%s: unable to retrieve UTF-8 string size.",
		 function );

		return( -1 );
	}
	return( 1 );
}

/* Retrieves the UTF-8 encoded name
 * The size should include the end of string character
 * Returns 1 if successful or -1 on error
 */
int libfsntfs_usn_record_get_utf8_name(
     libfsntfs_usn_record_t *usn_record,
     uint8_t *utf8_string,
     size_t utf8_string_size,
     libcerror_error_t **error )
{
	libfsntfs_internal_usn_record_t *internal_usn_record = NULL;
	static char *function                                = "libfsntfs_usn_record_get_utf8_name";

	if( usn_record == NULL )
	{
		libcerror_error_set(
		 error,
		 LIBCERROR_ERROR_DOMAIN_ARGUMENTS,
		 LIBCERROR_ARGUMENT_ERROR_INVALID_VALUE,
		 "%s: invalid USN record.",
		 function );

		return( -1 );
	}
	internal_usn_record = (libfsntfs_internal_usn_record_t *) usn_record;

	if( libuna_utf8_string_copy_from_utf16_stream(
	     utf8_string,
	     utf8_string_size,
	     internal_usn_record->name,
	     internal_usn_record->name_size,
	     LIBUNA_ENDIAN_LITTLE | LIBUNA_UTF16_STREAM_ALLOW_UNPAIRED_SURROGATE,
	     error ) != 1 )
	{
		libcerror_error_set(
		 error,
		 LIBCERROR_ERROR_DOMAIN_RUNTIME,
		 LIBCERROR_RUNTIME_ERROR_GET_FAILED,
		 "%s: unable to retrieve UTF-8 string.",
		 function );

		return( -1 );
	}
	return( 1 );
}

/* Retrieves the size of the UTF-16 encoded name
 * The returned size includes the end of string character
 * Returns 1 if successful or -1 on error
 */
int libfsntfs_usn_record_get_utf16_name_size(
     libfsntfs_usn_record_t *usn_record,
     size_t *utf16_string_size,
     libcerror_error_t **error )
{
	libfsntfs_internal_usn_record_t *internal_usn_record = NULL;
	static char *function                                = "libfsntfs_usn_record_get_utf16_name_size";

	if( usn_record == NULL )
	{
		libcerror_error_set(
		 error,
		 LIBCERROR_ERROR_DOMAIN_ARGUMENTS,
		 LIBCERROR_ARGUMENT_ERROR_INVALID_VALUE,
		 "%s: invalid USN record.",
		 function );

		return( -1 );
	}
	internal_usn_record = (libfsntfs_internal_usn_record_t *) usn_record;

	if( libuna_utf16_string_size_from_utf16_stream(
	     internal_usn_record->name,
	     internal_usn_record->name_size,
	     LIBUNA_ENDIAN_LITTLE | LIBUNA_UTF16_STREAM_ALLOW_UNPAIRED_SURROGATE,
	     utf16_string_size,
	     error ) != 1 )
	{
		libcerror_error_set(
		 error,
		 LIBCERROR_ERROR_DOMAIN_RUNTIME,
		 LIBCERROR_RUNTIME_ERROR_GET_FAILED,
		 "%s: unable to retrieve UTF-16 string size.",
		 function );

		return( -1 );
	}
	return( 1 );
}

/* Retrieves the UTF-16 encoded name
 * The size should include the end of string character
 * Returns 1 if successful or -1 on error
 */
int libfsntfs_usn_record_get_utf16_name(
     libfsntfs_usn_record_t *usn_record,
     uint16_t *utf16_string,
     size_t utf16_string_size,
     libcerror_error_t **error )
{
	libfsntfs_internal_usn_record_t *internal_usn_record = NULL;
	static char *function                                = "libfsntfs_usn_record_get_utf16_name";

	if( usn_record == NULL )
	{
		libcerror_error_set(
		 error,
		 LIBCERROR_ERROR_DOMAIN_ARGUMENTS,
		 LIBCERROR_ARGUMENT_ERROR_INVALID_VALUE,
		 "%s: invalid USN record.",
		 function );

		return( -1 );
	}
	internal_usn_record = (libfsntfs_internal_usn_record_t *) usn_record;

	if( libuna_utf16_string_copy_from_utf16_stream(
	     utf16_string,
	     utf16_string_size,
	     internal_usn_record->name,
	     internal_usn_record->name_size,
	     LIBUNA_ENDIAN_LITTLE | LIBUNA_UTF16_STREAM_ALLOW_UNPAIRED_SURROGATE,
	     error ) != 1 )
	{
		libcerror_error_set(
		 error,
		 LIBCERROR_ERROR_DOMAIN_RUNTIME,
		 LIBCERROR_RUNTIME_ERROR_GET_FAILED,
		 "%s: unable to retrieve UTF-16 string.",
		 function );

		return( -1 );
	}
	return( 1 );
}

/* Retrieves the file entry associated with the USN record
 * Returns 1 if successful, 0 if no matching file entry or -1 on error
 */
int libfsntfs_usn_record_get_file_entry(
     libfsntfs_usn_record_t *usn_record,
     libfsntfs_volume_t *volume,
     libfsntfs_file_entry_t **file_entry,
     libcerror_error_t **error )
{
	libfsntfs_internal_usn_record_t *internal_usn_record = NULL;
	static char *function                                = "libfsntfs_usn_record_get_file_entry";

	if( usn_record == NULL )
	{
		libcerror_error_set(
		 error,
		 LIBCERROR_ERROR_DOMAIN_ARGUMENTS,
		 LIBCERROR_ARGUMENT_ERROR_INVALID_VALUE,
		 "%s: invalid USN record.",
		 function );

		return( -1 );
	}
	internal_usn_record = (libfsntfs_internal_usn_record_t *) usn_record;

	if( volume == NULL )
	{
		libcerror_error_set(
		 error,
		 LIBCERROR_ERROR_DOMAIN_ARGUMENTS,
		 LIBCERROR_ARGUMENT_ERROR_INVALID_VALUE,
		 "%s: invalid volume.",
		 function );

		return( -1 );
	}
	if( file_entry == NULL )
	{
		libcerror_error_set(
		 error,
		 LIBCERROR_ERROR_DOMAIN_ARGUMENTS,
		 LIBCERROR_ARGUMENT_ERROR_INVALID_VALUE,
		 "%s: invalid file entry.",
		 function );

		return( -1 );
	}
	if( *file_entry != NULL )
	{
		libcerror_error_set(
		 error,
		 LIBCERROR_ERROR_DOMAIN_RUNTIME,
		 LIBCERROR_RUNTIME_ERROR_VALUE_ALREADY_SET,
		 "%s: invalid file entry value already set.",
		 function );

		return( -1 );
	}
	return( libfsntfs_internal_usn_record_get_file_entry_by_reference(
	         internal_usn_record->file_reference,
	         volume,
	         file_entry,
	         error ) );
}

/* Retrieves the parent file entry associated with the USN record
 * Returns 1 if successful, 0 if no matching file entry or -1 on error
 */
int libfsntfs_usn_record_get_parent_file_entry(
     libfsntfs_usn_record_t *usn_record,
     libfsntfs_volume_t *volume,
     libfsntfs_file_entry_t **parent_file_entry,
     libcerror_error_t **error )
{
	libfsntfs_internal_usn_record_t *internal_usn_record = NULL;
	static char *function                                = "libfsntfs_usn_record_get_parent_file_entry";

	if( usn_record == NULL )
	{
		libcerror_error_set(
		 error,
		 LIBCERROR_ERROR_DOMAIN_ARGUMENTS,
		 LIBCERROR_ARGUMENT_ERROR_INVALID_VALUE,
		 "%s: invalid USN record.",
		 function );

		return( -1 );
	}
	internal_usn_record = (libfsntfs_internal_usn_record_t *) usn_record;

	if( volume == NULL )
	{
		libcerror_error_set(
		 error,
		 LIBCERROR_ERROR_DOMAIN_ARGUMENTS,
		 LIBCERROR_ARGUMENT_ERROR_INVALID_VALUE,
		 "%s: invalid volume.",
		 function );

		return( -1 );
	}
	if( parent_file_entry == NULL )
	{
		libcerror_error_set(
		 error,
		 LIBCERROR_ERROR_DOMAIN_ARGUMENTS,
		 LIBCERROR_ARGUMENT_ERROR_INVALID_VALUE,
		 "%s: invalid parent file entry.",
		 function );

		return( -1 );
	}
	if( *parent_file_entry != NULL )
	{
		libcerror_error_set(
		 error,
		 LIBCERROR_ERROR_DOMAIN_RUNTIME,
		 LIBCERROR_RUNTIME_ERROR_VALUE_ALREADY_SET,
		 "%s: invalid parent file entry value already set.",
		 function );

		return( -1 );
	}
	return( libfsntfs_internal_usn_record_get_file_entry_by_reference(
	         internal_usn_record->parent_file_reference,
	         volume,
	         parent_file_entry,
	         error ) );
}

/* Retrieves the size of the UTF-8 encoded path
 * The path is constructed from the parent path and the name in the USN record.
 * The returned size includes the end of string character
 * Returns 1 if successful, 0 if the path is not available or -1 on error
 */
int libfsntfs_usn_record_get_utf8_path_size(
     libfsntfs_usn_record_t *usn_record,
     libfsntfs_volume_t *volume,
     size_t *utf8_string_size,
     libcerror_error_t **error )
{
	libfsntfs_internal_usn_record_t *internal_usn_record = NULL;
	static char *function                                = "libfsntfs_usn_record_get_utf8_path_size";
	size_t name_size                                     = 0;
	size_t parent_path_size                              = 0;
	size_t path_size                                     = 0;
	uint64_t parent_mft_entry_index                      = 0;
	int result                                           = 0;

	if( usn_record == NULL )
	{
		libcerror_error_set(
		 error,
		 LIBCERROR_ERROR_DOMAIN_ARGUMENTS,
		 LIBCERROR_ARGUMENT_ERROR_INVALID_VALUE,
		 "%s: invalid USN record.",
		 function );

		return( -1 );
	}
	internal_usn_record = (libfsntfs_internal_usn_record_t *) usn_record;

	if( volume == NULL )
	{
		libcerror_error_set(
		 error,
		 LIBCERROR_ERROR_DOMAIN_ARGUMENTS,
		 LIBCERROR_ARGUMENT_ERROR_INVALID_VALUE,
		 "%s: invalid volume.",
		 function );

		return( -1 );
	}
	if( utf8_string_size == NULL )
	{
		libcerror_error_set(
		 error,
		 LIBCERROR_ERROR_DOMAIN_ARGUMENTS,
		 LIBCERROR_ARGUMENT_ERROR_INVALID_VALUE,
		 "%s: invalid UTF-8 string size.",
		 function );

		return( -1 );
	}
	*utf8_string_size = 0;

	result = libfsntfs_internal_usn_record_get_parent_path_size(
	          internal_usn_record,
	          volume,
	          &parent_path_size,
	          error );

	if( result == -1 )
	{
		libcerror_error_set(
		 error,
		 LIBCERROR_ERROR_DOMAIN_RUNTIME,
		 LIBCERROR_RUNTIME_ERROR_GET_FAILED,
		 "%s: unable to retrieve parent path size.",
		 function );

		return( -1 );
	}
	if( result == 0 )
	{
		return( 0 );
	}
	if( libfsntfs_usn_record_get_utf8_name_size(
	     usn_record,
	     &name_size,
	     error ) != 1 )
	{
		libcerror_error_set(
		 error,
		 LIBCERROR_ERROR_DOMAIN_RUNTIME,
		 LIBCERROR_RUNTIME_ERROR_GET_FAILED,
		 "%s: unable to retrieve name size.",
		 function );

		return( -1 );
	}
	if( parent_path_size == 0 )
	{
		return( 0 );
	}
	path_size = parent_path_size - 1;
	parent_mft_entry_index = internal_usn_record->parent_file_reference & 0x0000ffffffffffffUL;

	/* The parent path does not necessarily end with a separator.
	 */
	if( ( parent_mft_entry_index != LIBFSNTFS_MFT_ENTRY_INDEX_ROOT_DIRECTORY )
	 && ( path_size > 0 )
	 && ( name_size > 0 ) )
	{
		/* The implementation below always prepends the separator when
		 * constructing the path in libfsntfs_usn_record_get_utf8_path.
		 */
		path_size += 1;
	}
	path_size += name_size;

	*utf8_string_size = path_size;

	return( 1 );
}

/* Retrieves the UTF-8 encoded path
 * The path is constructed from the parent path and the name in the USN record.
 * The size should include the end of string character
 * Returns 1 if successful, 0 if the path is not available or -1 on error
 */
int libfsntfs_usn_record_get_utf8_path(
     libfsntfs_usn_record_t *usn_record,
     libfsntfs_volume_t *volume,
     uint8_t *utf8_string,
     size_t utf8_string_size,
     libcerror_error_t **error )
{
	libfsntfs_file_entry_t *parent_file_entry            = NULL;
	libfsntfs_internal_usn_record_t *internal_usn_record = NULL;
	static char *function                                = "libfsntfs_usn_record_get_utf8_path";
	size_t name_size                                     = 0;
	size_t parent_path_size                              = 0;
	size_t path_size                                     = 0;
	uint64_t parent_mft_entry_index                      = 0;
	int attribute_index                                  = 0;
	int result                                           = 0;

	if( usn_record == NULL )
	{
		libcerror_error_set(
		 error,
		 LIBCERROR_ERROR_DOMAIN_ARGUMENTS,
		 LIBCERROR_ARGUMENT_ERROR_INVALID_VALUE,
		 "%s: invalid USN record.",
		 function );

		return( -1 );
	}
	internal_usn_record = (libfsntfs_internal_usn_record_t *) usn_record;

	if( volume == NULL )
	{
		libcerror_error_set(
		 error,
		 LIBCERROR_ERROR_DOMAIN_ARGUMENTS,
		 LIBCERROR_ARGUMENT_ERROR_INVALID_VALUE,
		 "%s: invalid volume.",
		 function );

		return( -1 );
	}
	if( utf8_string == NULL )
	{
		libcerror_error_set(
		 error,
		 LIBCERROR_ERROR_DOMAIN_ARGUMENTS,
		 LIBCERROR_ARGUMENT_ERROR_INVALID_VALUE,
		 "%s: invalid UTF-8 string.",
		 function );

		return( -1 );
	}
	result = libfsntfs_usn_record_get_utf8_path_size(
	          usn_record,
	          volume,
	          &path_size,
	          error );

	if( result == -1 )
	{
		libcerror_error_set(
		 error,
		 LIBCERROR_ERROR_DOMAIN_RUNTIME,
		 LIBCERROR_RUNTIME_ERROR_GET_FAILED,
		 "%s: unable to retrieve path size.",
		 function );

		return( -1 );
	}
	if( result == 0 )
	{
		return( 0 );
	}
	if( utf8_string_size < path_size )
	{
		libcerror_error_set(
		 error,
		 LIBCERROR_ERROR_DOMAIN_ARGUMENTS,
		 LIBCERROR_ARGUMENT_ERROR_VALUE_TOO_SMALL,
		 "%s: UTF-8 string size value too small.",
		 function );

		return( -1 );
	}
	parent_mft_entry_index = internal_usn_record->parent_file_reference & 0x0000ffffffffffffUL;

	if( parent_mft_entry_index == LIBFSNTFS_MFT_ENTRY_INDEX_ROOT_DIRECTORY )
	{
		utf8_string[ 0 ] = (uint8_t) LIBFSNTFS_SEPARATOR;
		utf8_string[ 1 ] = 0;

		parent_path_size = 2;
	}
	else
	{
		result = libfsntfs_internal_usn_record_get_file_entry_by_reference(
		          internal_usn_record->parent_file_reference,
		          volume,
		          &parent_file_entry,
		          error );

		if( result == -1 )
		{
			libcerror_error_set(
			 error,
			 LIBCERROR_ERROR_DOMAIN_RUNTIME,
			 LIBCERROR_RUNTIME_ERROR_GET_FAILED,
			 "%s: unable to retrieve parent file entry.",
			 function );

			return( -1 );
		}
		if( result == 0 )
		{
			return( 0 );
		}
		result = libfsntfs_internal_usn_record_get_file_name_attribute_index(
		          parent_file_entry,
		          &attribute_index,
		          error );

		if( result != 1 )
		{
			libcerror_error_set(
			 error,
			 LIBCERROR_ERROR_DOMAIN_RUNTIME,
			 LIBCERROR_RUNTIME_ERROR_GET_FAILED,
			 "%s: unable to retrieve parent name attribute index.",
			 function );

			goto on_error;
		}
		result = libfsntfs_file_entry_get_utf8_path_hint_size(
		          parent_file_entry,
		          attribute_index,
		          &parent_path_size,
		          error );

		if( result != 1 )
		{
			libcerror_error_set(
			 error,
			 LIBCERROR_ERROR_DOMAIN_RUNTIME,
			 LIBCERROR_RUNTIME_ERROR_GET_FAILED,
			 "%s: unable to retrieve parent path size.",
			 function );

			goto on_error;
		}
		if( parent_path_size > utf8_string_size )
		{
			libcerror_error_set(
			 error,
			 LIBCERROR_ERROR_DOMAIN_RUNTIME,
			 LIBCERROR_RUNTIME_ERROR_VALUE_OUT_OF_BOUNDS,
			 "%s: parent path size value out of bounds.",
			 function );

			goto on_error;
		}
		result = libfsntfs_file_entry_get_utf8_path_hint(
		          parent_file_entry,
		          attribute_index,
		          utf8_string,
		          utf8_string_size,
		          error );

		if( result != 1 )
		{
			libcerror_error_set(
			 error,
			 LIBCERROR_ERROR_DOMAIN_RUNTIME,
			 LIBCERROR_RUNTIME_ERROR_GET_FAILED,
			 "%s: unable to retrieve parent path.",
			 function );

			goto on_error;
		}
		if( libfsntfs_file_entry_free(
		     &parent_file_entry,
		     error ) != 1 )
		{
			libcerror_error_set(
			 error,
			 LIBCERROR_ERROR_DOMAIN_RUNTIME,
			 LIBCERROR_RUNTIME_ERROR_FINALIZE_FAILED,
			 "%s: unable to free parent file entry.",
			 function );

			return( -1 );
		}
	}
	if( libfsntfs_usn_record_get_utf8_name_size(
	     usn_record,
	     &name_size,
	     error ) != 1 )
	{
		libcerror_error_set(
		 error,
		 LIBCERROR_ERROR_DOMAIN_RUNTIME,
		 LIBCERROR_RUNTIME_ERROR_GET_FAILED,
		 "%s: unable to retrieve name size.",
		 function );

		return( -1 );
	}
	if( parent_path_size == 0 )
	{
		return( 0 );
	}
	parent_path_size -= 1;
	if( ( parent_mft_entry_index != LIBFSNTFS_MFT_ENTRY_INDEX_ROOT_DIRECTORY )
	 && ( parent_path_size > 0 )
	 && ( parent_path_size < utf8_string_size ) )
	{
		utf8_string[ parent_path_size++ ] = (uint8_t) LIBFSNTFS_SEPARATOR;
	}
	if( libfsntfs_usn_record_get_utf8_name(
	     usn_record,
	     &( utf8_string[ parent_path_size ] ),
	     utf8_string_size - parent_path_size,
	     error ) != 1 )
	{
		libcerror_error_set(
		 error,
		 LIBCERROR_ERROR_DOMAIN_RUNTIME,
		 LIBCERROR_RUNTIME_ERROR_GET_FAILED,
		 "%s: unable to retrieve name.",
		 function );

		return( -1 );
	}
	return( 1 );

on_error:
	if( parent_file_entry != NULL )
	{
		libfsntfs_file_entry_free(
		 &parent_file_entry,
		 NULL );
	}
	return( -1 );
}
