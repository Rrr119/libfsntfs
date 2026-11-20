/*
 * Library usn_change_journal type test program
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
#include <file_stream.h>
#include <narrow_string.h>
#include <types.h>

#if defined( HAVE_STDLIB_H ) || defined( WINAPI )
#include <stdlib.h>
#endif

#include "fsntfs_test_libbfio.h"
#include "fsntfs_test_libcerror.h"
#include "fsntfs_test_libfsntfs.h"
#include "fsntfs_test_macros.h"
#include "fsntfs_test_memory.h"
#include "fsntfs_test_unused.h"

#include "../libfsntfs/libfsntfs_directory_entry.h"
#include "../libfsntfs/libfsntfs_io_handle.h"
#include "../libfsntfs/libfsntfs_usn_change_journal.h"

static const uint8_t fsntfs_test_usn_record_data[ 80 ] = {
	0x50, 0x00, 0x00, 0x00, 0x02, 0x00, 0x00, 0x00, 0x40, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x01, 0x00, 0x05, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x00,
	0x88, 0x77, 0x66, 0x55, 0x44, 0x33, 0x22, 0x11, 0x00, 0x80, 0x20, 0x9b,
	0xcb, 0x82, 0xd8, 0x01, 0x00, 0x21, 0x00, 0x80, 0x02, 0x00, 0x00, 0x00,
	0x34, 0x12, 0x00, 0x00, 0x20, 0x00, 0x00, 0x00, 0x12, 0x00, 0x3c, 0x00,
	0x68, 0x00, 0x65, 0x00, 0x6c, 0x00, 0x6c, 0x00, 0x6f, 0x00, 0x2e, 0x00,
	0x74, 0x00, 0x78, 0x00, 0x74, 0x00, 0x00, 0x00
};

/* Tests the libfsntfs_usn_record_initialize and value functions
 * Returns 1 if successful or 0 if not
 */
int fsntfs_test_usn_record_initialize(
     void )
{
	uint8_t utf8_name[ 16 ];
	uint16_t utf16_name[ 16 ];

	libcerror_error_t *error         = NULL;
	libfsntfs_usn_record_t *record   = NULL;
	size_t utf8_name_size            = 0;
	size_t utf16_name_size           = 0;
	uint64_t file_reference         = 0;
	uint64_t parent_file_reference  = 0;
	uint64_t update_sequence_number = 0;
	uint64_t update_time            = 0;
	uint32_t file_attribute_flags   = 0;
	uint32_t record_size            = 0;
	uint32_t security_identifier    = 0;
	uint32_t source_flags           = 0;
	uint32_t reason_flags           = 0;
	uint16_t major_version          = 0;
	uint16_t minor_version          = 0;
	int result                      = 0;

	result = libfsntfs_usn_record_initialize(
	          &record,
	          fsntfs_test_usn_record_data,
	          80,
	          &error );

	FSNTFS_TEST_ASSERT_EQUAL_INT(
	 "result",
	 result,
	 1 );

	FSNTFS_TEST_ASSERT_IS_NOT_NULL(
	 "record",
	 record );

	FSNTFS_TEST_ASSERT_IS_NULL(
	 "error",
	 error );

	result = libfsntfs_usn_record_get_size(
	          record,
	          &record_size,
	          &error );

	FSNTFS_TEST_ASSERT_EQUAL_INT(
	 "result",
	 result,
	 1 );

	FSNTFS_TEST_ASSERT_EQUAL_UINT32(
	 "record_size",
	 record_size,
	 (uint32_t) 80 );

	result = libfsntfs_usn_record_get_version(
	          record,
	          &major_version,
	          &minor_version,
	          &error );

	FSNTFS_TEST_ASSERT_EQUAL_INT(
	 "result",
	 result,
	 1 );

	FSNTFS_TEST_ASSERT_EQUAL_UINT16(
	 "major_version",
	 major_version,
	 (uint16_t) 2 );

	FSNTFS_TEST_ASSERT_EQUAL_UINT16(
	 "minor_version",
	 minor_version,
	 (uint16_t) 0 );

	result = libfsntfs_usn_record_get_file_reference(
	          record,
	          &file_reference,
	          &error );

	FSNTFS_TEST_ASSERT_EQUAL_INT(
	 "result",
	 result,
	 1 );

	FSNTFS_TEST_ASSERT_EQUAL_UINT64(
	 "file_reference",
	 file_reference,
	 (uint64_t) 0x0001000000000040UL );

	result = libfsntfs_usn_record_get_parent_file_reference(
	          record,
	          &parent_file_reference,
	          &error );

	FSNTFS_TEST_ASSERT_EQUAL_INT(
	 "result",
	 result,
	 1 );

	FSNTFS_TEST_ASSERT_EQUAL_UINT64(
	 "parent_file_reference",
	 parent_file_reference,
	 (uint64_t) 0x0001000000000005UL );

	result = libfsntfs_usn_record_get_update_sequence_number(
	          record,
	          &update_sequence_number,
	          &error );

	FSNTFS_TEST_ASSERT_EQUAL_INT(
	 "result",
	 result,
	 1 );

	FSNTFS_TEST_ASSERT_EQUAL_UINT64(
	 "update_sequence_number",
	 update_sequence_number,
	 (uint64_t) 0x1122334455667788UL );

	result = libfsntfs_usn_record_get_update_time(
	          record,
	          &update_time,
	          &error );

	FSNTFS_TEST_ASSERT_EQUAL_INT(
	 "result",
	 result,
	 1 );

	FSNTFS_TEST_ASSERT_EQUAL_UINT64(
	 "update_time",
	 update_time,
	 (uint64_t) 133000000000000000UL );

	result = libfsntfs_usn_record_get_update_reason_flags(
	          record,
	          &reason_flags,
	          &error );

	FSNTFS_TEST_ASSERT_EQUAL_INT(
	 "result",
	 result,
	 1 );

	FSNTFS_TEST_ASSERT_EQUAL_UINT32(
	 "reason_flags",
	 reason_flags,
	 (uint32_t) 0x80002100UL );

	result = libfsntfs_usn_record_get_update_source_flags(
	          record,
	          &source_flags,
	          &error );

	FSNTFS_TEST_ASSERT_EQUAL_INT(
	 "result",
	 result,
	 1 );

	FSNTFS_TEST_ASSERT_EQUAL_UINT32(
	 "source_flags",
	 source_flags,
	 (uint32_t) 0x00000002UL );

	result = libfsntfs_usn_record_get_security_descriptor_identifier(
	          record,
	          &security_identifier,
	          &error );

	FSNTFS_TEST_ASSERT_EQUAL_INT(
	 "result",
	 result,
	 1 );

	FSNTFS_TEST_ASSERT_EQUAL_UINT32(
	 "security_identifier",
	 security_identifier,
	 (uint32_t) 0x00001234UL );

	result = libfsntfs_usn_record_get_file_attribute_flags(
	          record,
	          &file_attribute_flags,
	          &error );

	FSNTFS_TEST_ASSERT_EQUAL_INT(
	 "result",
	 result,
	 1 );

	FSNTFS_TEST_ASSERT_EQUAL_UINT32(
	 "file_attribute_flags",
	 file_attribute_flags,
	 (uint32_t) 0x00000020UL );

	result = libfsntfs_usn_record_get_utf8_name_size(
	          record,
	          &utf8_name_size,
	          &error );

	FSNTFS_TEST_ASSERT_EQUAL_INT(
	 "result",
	 result,
	 1 );

	FSNTFS_TEST_ASSERT_EQUAL_SIZE(
	 "utf8_name_size",
	 utf8_name_size,
	 (size_t) 10 );

	result = libfsntfs_usn_record_get_utf8_name(
	          record,
	          utf8_name,
	          16,
	          &error );

	FSNTFS_TEST_ASSERT_EQUAL_INT(
	 "result",
	 result,
	 1 );

	FSNTFS_TEST_ASSERT_IS_NULL(
	 "error",
	 error );

	FSNTFS_TEST_ASSERT_EQUAL_INT(
	 "name compare",
	 narrow_string_compare(
	  (char *) utf8_name,
	  "hello.txt",
	  9 ),
	 0 );

	result = libfsntfs_usn_record_get_utf16_name_size(
	          record,
	          &utf16_name_size,
	          &error );

	FSNTFS_TEST_ASSERT_EQUAL_INT(
	 "result",
	 result,
	 1 );

	FSNTFS_TEST_ASSERT_EQUAL_SIZE(
	 "utf16_name_size",
	 utf16_name_size,
	 (size_t) 10 );

	result = libfsntfs_usn_record_get_utf16_name(
	          record,
	          utf16_name,
	          16,
	          &error );

	FSNTFS_TEST_ASSERT_EQUAL_INT(
	 "result",
	 result,
	 1 );

	FSNTFS_TEST_ASSERT_EQUAL_UINT16(
	 "utf16_name[ 0 ]",
	 utf16_name[ 0 ],
	 (uint16_t) 'h' );

	FSNTFS_TEST_ASSERT_EQUAL_UINT16(
	 "utf16_name[ 8 ]",
	 utf16_name[ 8 ],
	 (uint16_t) 't' );

	result = libfsntfs_usn_record_free(
	          &record,
	          &error );

	FSNTFS_TEST_ASSERT_EQUAL_INT(
	 "result",
	 result,
	 1 );

	FSNTFS_TEST_ASSERT_IS_NULL(
	 "record",
	 record );

	FSNTFS_TEST_ASSERT_IS_NULL(
	 "error",
	 error );

	/* Test error cases
	 */
	result = libfsntfs_usn_record_initialize(
	          NULL,
	          fsntfs_test_usn_record_data,
	          80,
	          &error );

	FSNTFS_TEST_ASSERT_EQUAL_INT(
	 "result",
	 result,
	 -1 );

	FSNTFS_TEST_ASSERT_IS_NOT_NULL(
	 "error",
	 error );

	libcerror_error_free(
	 &error );

	result = libfsntfs_usn_record_initialize(
	          &record,
	          fsntfs_test_usn_record_data,
	          59,
	          &error );

	FSNTFS_TEST_ASSERT_EQUAL_INT(
	 "result",
	 result,
	 -1 );

	FSNTFS_TEST_ASSERT_IS_NOT_NULL(
	 "error",
	 error );

	libcerror_error_free(
	 &error );

	return( 1 );

on_error:
	if( record != NULL )
	{
		libfsntfs_usn_record_free(
		 &record,
		 NULL );
	}
	if( error != NULL )
	{
		libcerror_error_free(
		 &error );
	}
	return( 0 );
}


/* TODO implement */
#ifdef TODO

#if defined( __GNUC__ ) && !defined( LIBFSNTFS_DLL_IMPORT )

/* Tests the libfsntfs_usn_change_journal_initialize function
 * Returns 1 if successful or 0 if not
 */
int fsntfs_test_usn_change_journal_initialize(
     void )
{
	libbfio_handle_t *file_io_handle                   = NULL;
	libcerror_error_t *error                           = NULL;
	libfsntfs_attribute_t *data_attribute              = NULL;
	libfsntfs_directory_entry_t *directory_entry       = NULL;
	libfsntfs_io_handle_t *io_handle                   = NULL;
	libfsntfs_usn_change_journal_t *usn_change_journal = NULL;
	int result                                         = 0;

#if defined( HAVE_FSNTFS_TEST_MEMORY )
	int number_of_malloc_fail_tests                    = 1;
	int number_of_memset_fail_tests                    = 1;
	int test_number                                    = 0;
#endif

	/* Initialize test
	 */
	result = libfsntfs_io_handle_initialize(
	          &io_handle,
	          &error );

	FSNTFS_TEST_ASSERT_EQUAL_INT(
	 "result",
	 result,
	 1 );

	FSNTFS_TEST_ASSERT_IS_NOT_NULL(
	 "io_handle",
	 io_handle );

	FSNTFS_TEST_ASSERT_IS_NULL(
	 "error",
	 error );

	io_handle->cluster_block_size = 4096;

	/* Test regular cases
	 */
	result = libfsntfs_usn_change_journal_initialize(
	          &usn_change_journal,
	          io_handle,
	          file_io_handle,
	          directory_entry,
	          data_attribute,
	          &error );

	FSNTFS_TEST_ASSERT_EQUAL_INT(
	 "result",
	 result,
	 1 );

	FSNTFS_TEST_ASSERT_IS_NOT_NULL(
	 "usn_change_journal",
	 usn_change_journal );

	FSNTFS_TEST_ASSERT_IS_NULL(
	 "error",
	 error );

	result = libfsntfs_usn_change_journal_free(
	          &usn_change_journal,
	          &error );

	FSNTFS_TEST_ASSERT_EQUAL_INT(
	 "result",
	 result,
	 1 );

	FSNTFS_TEST_ASSERT_IS_NULL(
	 "usn_change_journal",
	 usn_change_journal );

	FSNTFS_TEST_ASSERT_IS_NULL(
	 "error",
	 error );

	/* Test error cases
	 */
	result = libfsntfs_usn_change_journal_initialize(
	          NULL,
	          io_handle,
	          file_io_handle,
	          directory_entry,
	          data_attribute,
	          &error );

	FSNTFS_TEST_ASSERT_EQUAL_INT(
	 "result",
	 result,
	 -1 );

	FSNTFS_TEST_ASSERT_IS_NOT_NULL(
	 "error",
	 error );

	libcerror_error_free(
	 &error );

	usn_change_journal = (libfsntfs_usn_change_journal_t *) 0x12345678UL;

	result = libfsntfs_usn_change_journal_initialize(
	          &usn_change_journal,
	          io_handle,
	          file_io_handle,
	          directory_entry,
	          data_attribute,
	          &error );

	usn_change_journal = NULL;

	FSNTFS_TEST_ASSERT_EQUAL_INT(
	 "result",
	 result,
	 -1 );

	FSNTFS_TEST_ASSERT_IS_NOT_NULL(
	 "error",
	 error );

	libcerror_error_free(
	 &error );

#if defined( HAVE_FSNTFS_TEST_MEMORY )

	for( test_number = 0;
	     test_number < number_of_malloc_fail_tests;
	     test_number++ )
	{
		/* Test libfsntfs_usn_change_journal_initialize with malloc failing
		 */
		fsntfs_test_malloc_attempts_before_fail = test_number;

		result = libfsntfs_usn_change_journal_initialize(
		          &usn_change_journal,
		          io_handle,
		          file_io_handle,
		          directory_entry,
		          data_attribute,
		          &error );

		if( fsntfs_test_malloc_attempts_before_fail != -1 )
		{
			fsntfs_test_malloc_attempts_before_fail = -1;

			if( usn_change_journal != NULL )
			{
				libfsntfs_usn_change_journal_free(
				 &usn_change_journal,
				 NULL );
			}
		}
		else
		{
			FSNTFS_TEST_ASSERT_EQUAL_INT(
			 "result",
			 result,
			 -1 );

			FSNTFS_TEST_ASSERT_IS_NULL(
			 "usn_change_journal",
			 usn_change_journal );

			FSNTFS_TEST_ASSERT_IS_NOT_NULL(
			 "error",
			 error );

			libcerror_error_free(
			 &error );
		}
	}
	for( test_number = 0;
	     test_number < number_of_memset_fail_tests;
	     test_number++ )
	{
		/* Test libfsntfs_usn_change_journal_initialize with memset failing
		 */
		fsntfs_test_memset_attempts_before_fail = test_number;

		result = libfsntfs_usn_change_journal_initialize(
		          &usn_change_journal,
		          io_handle,
		          file_io_handle,
		          directory_entry,
		          data_attribute,
		          &error );

		if( fsntfs_test_memset_attempts_before_fail != -1 )
		{
			fsntfs_test_memset_attempts_before_fail = -1;

			if( usn_change_journal != NULL )
			{
				libfsntfs_usn_change_journal_free(
				 &usn_change_journal,
				 NULL );
			}
		}
		else
		{
			FSNTFS_TEST_ASSERT_EQUAL_INT(
			 "result",
			 result,
			 -1 );

			FSNTFS_TEST_ASSERT_IS_NULL(
			 "usn_change_journal",
			 usn_change_journal );

			FSNTFS_TEST_ASSERT_IS_NOT_NULL(
			 "error",
			 error );

			libcerror_error_free(
			 &error );
		}
	}
#endif /* defined( HAVE_FSNTFS_TEST_MEMORY ) */

	/* Clean up
	 */
	result = libfsntfs_internal_attribute_free(
	          (libfsntfs_internal_attribute_t **) &data_attribute,
	          &error );

	FSNTFS_TEST_ASSERT_EQUAL_INT(
	 "result",
	 result,
	 1 );

	FSNTFS_TEST_ASSERT_IS_NULL(
	 "data_attribute",
	 data_attribute );

	FSNTFS_TEST_ASSERT_IS_NULL(
	 "error",
	 error );

	result = libfsntfs_io_handle_free(
	          &io_handle,
	          &error );

	FSNTFS_TEST_ASSERT_EQUAL_INT(
	 "result",
	 result,
	 1 );

	FSNTFS_TEST_ASSERT_IS_NULL(
	 "io_handle",
	 io_handle );

	FSNTFS_TEST_ASSERT_IS_NULL(
	 "error",
	 error );

	return( 1 );

on_error:
	if( error != NULL )
	{
		libcerror_error_free(
		 &error );
	}
	if( usn_change_journal != NULL )
	{
		libfsntfs_usn_change_journal_free(
		 &usn_change_journal,
		 NULL );
	}
	if( io_handle != NULL )
	{
		libfsntfs_io_handle_free(
		 &io_handle,
		 NULL );
	}
	return( 0 );
}

#endif /* defined( __GNUC__ ) && !defined( LIBFSNTFS_DLL_IMPORT ) */
#endif /* TODO */

/* Tests the libfsntfs_usn_change_journal_free function
 * Returns 1 if successful or 0 if not
 */
int fsntfs_test_usn_change_journal_free(
     void )
{
	libcerror_error_t *error = NULL;
	int result               = 0;

	/* Test error cases
	 */
	result = libfsntfs_usn_change_journal_free(
	          NULL,
	          &error );

	FSNTFS_TEST_ASSERT_EQUAL_INT(
	 "result",
	 result,
	 -1 );

	FSNTFS_TEST_ASSERT_IS_NOT_NULL(
	 "error",
	 error );

	libcerror_error_free(
	 &error );

	return( 1 );

on_error:
	if( error != NULL )
	{
		libcerror_error_free(
		 &error );
	}
	return( 0 );
}

#if defined( __GNUC__ ) && !defined( LIBFSNTFS_DLL_IMPORT )

#endif /* defined( __GNUC__ ) && !defined( LIBFSNTFS_DLL_IMPORT ) */

/* The main program
 */
#if defined( HAVE_WIDE_SYSTEM_CHARACTER )
int wmain(
     int argc FSNTFS_TEST_ATTRIBUTE_UNUSED,
     wchar_t * const argv[] FSNTFS_TEST_ATTRIBUTE_UNUSED )
#else
int main(
     int argc FSNTFS_TEST_ATTRIBUTE_UNUSED,
     char * const argv[] FSNTFS_TEST_ATTRIBUTE_UNUSED )
#endif
{
	FSNTFS_TEST_UNREFERENCED_PARAMETER( argc )
	FSNTFS_TEST_UNREFERENCED_PARAMETER( argv )

#if defined( __GNUC__ ) && !defined( LIBFSNTFS_DLL_IMPORT )

	/* TODO: add tests for libfsntfs_usn_change_journal_initialize */

#endif /* defined( __GNUC__ ) && !defined( LIBFSNTFS_DLL_IMPORT ) */

	FSNTFS_TEST_RUN(
	 "libfsntfs_usn_change_journal_free",
	 fsntfs_test_usn_change_journal_free );

	FSNTFS_TEST_RUN(
	 "libfsntfs_usn_record_initialize",
	 fsntfs_test_usn_record_initialize );

#if defined( __GNUC__ ) && !defined( LIBFSNTFS_DLL_IMPORT )

	/* TODO: add tests for libfsntfs_usn_change_journal_get_offset */

	/* TODO: add tests for libfsntfs_usn_change_journal_read_usn_record */

#endif /* defined( __GNUC__ ) && !defined( LIBFSNTFS_DLL_IMPORT ) */

	return( EXIT_SUCCESS );

on_error:
	return( EXIT_FAILURE );
}

