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

#if !defined( _LIBFSNTFS_USN_RECORD_H )
#define _LIBFSNTFS_USN_RECORD_H

#include <common.h>
#include <types.h>

#include "libfsntfs_extern.h"
#include "libfsntfs_libcerror.h"
#include "libfsntfs_types.h"

#if defined( __cplusplus )
extern "C" {
#endif

typedef struct libfsntfs_internal_usn_record libfsntfs_internal_usn_record_t;

struct libfsntfs_internal_usn_record
{
	/* The record size
	 */
	uint32_t record_size;

	/* The major version
	 */
	uint16_t major_version;

	/* The minor version
	 */
	uint16_t minor_version;

	/* The file reference
	 */
	uint64_t file_reference;

	/* The parent file reference
	 */
	uint64_t parent_file_reference;

	/* The update sequence number
	 */
	uint64_t update_sequence_number;

	/* The update time
	 */
	uint64_t update_time;

	/* The update reason flags
	 */
	uint32_t update_reason_flags;

	/* The update source flags
	 */
	uint32_t update_source_flags;

	/* The security descriptor identifier
	 */
	uint32_t security_descriptor_identifier;

	/* The file attribute flags
	 */
	uint32_t file_attribute_flags;

	/* The UTF-16 little-endian formatted name
	 */
	uint8_t *name;

	/* The name size
	 */
	size_t name_size;
};

LIBFSNTFS_EXTERN \
int libfsntfs_usn_record_initialize(
     libfsntfs_usn_record_t **usn_record,
     const uint8_t *data,
     size_t data_size,
     libcerror_error_t **error );

LIBFSNTFS_EXTERN \
int libfsntfs_usn_record_free(
     libfsntfs_usn_record_t **usn_record,
     libcerror_error_t **error );

LIBFSNTFS_EXTERN \
int libfsntfs_usn_record_get_size(
     libfsntfs_usn_record_t *usn_record,
     uint32_t *record_size,
     libcerror_error_t **error );

LIBFSNTFS_EXTERN \
int libfsntfs_usn_record_get_version(
     libfsntfs_usn_record_t *usn_record,
     uint16_t *major_version,
     uint16_t *minor_version,
     libcerror_error_t **error );

LIBFSNTFS_EXTERN \
int libfsntfs_usn_record_get_file_reference(
     libfsntfs_usn_record_t *usn_record,
     uint64_t *file_reference,
     libcerror_error_t **error );

LIBFSNTFS_EXTERN \
int libfsntfs_usn_record_get_parent_file_reference(
     libfsntfs_usn_record_t *usn_record,
     uint64_t *parent_file_reference,
     libcerror_error_t **error );

LIBFSNTFS_EXTERN \
int libfsntfs_usn_record_get_update_sequence_number(
     libfsntfs_usn_record_t *usn_record,
     uint64_t *update_sequence_number,
     libcerror_error_t **error );

LIBFSNTFS_EXTERN \
int libfsntfs_usn_record_get_update_time(
     libfsntfs_usn_record_t *usn_record,
     uint64_t *filetime,
     libcerror_error_t **error );

LIBFSNTFS_EXTERN \
int libfsntfs_usn_record_get_update_reason_flags(
     libfsntfs_usn_record_t *usn_record,
     uint32_t *update_reason_flags,
     libcerror_error_t **error );

LIBFSNTFS_EXTERN \
int libfsntfs_usn_record_get_update_source_flags(
     libfsntfs_usn_record_t *usn_record,
     uint32_t *update_source_flags,
     libcerror_error_t **error );

LIBFSNTFS_EXTERN \
int libfsntfs_usn_record_get_security_descriptor_identifier(
     libfsntfs_usn_record_t *usn_record,
     uint32_t *security_descriptor_identifier,
     libcerror_error_t **error );

LIBFSNTFS_EXTERN \
int libfsntfs_usn_record_get_file_attribute_flags(
     libfsntfs_usn_record_t *usn_record,
     uint32_t *file_attribute_flags,
     libcerror_error_t **error );

LIBFSNTFS_EXTERN \
int libfsntfs_usn_record_get_utf8_name_size(
     libfsntfs_usn_record_t *usn_record,
     size_t *utf8_string_size,
     libcerror_error_t **error );

LIBFSNTFS_EXTERN \
int libfsntfs_usn_record_get_utf8_name(
     libfsntfs_usn_record_t *usn_record,
     uint8_t *utf8_string,
     size_t utf8_string_size,
     libcerror_error_t **error );

LIBFSNTFS_EXTERN \
int libfsntfs_usn_record_get_utf16_name_size(
     libfsntfs_usn_record_t *usn_record,
     size_t *utf16_string_size,
     libcerror_error_t **error );

LIBFSNTFS_EXTERN \
int libfsntfs_usn_record_get_utf16_name(
     libfsntfs_usn_record_t *usn_record,
     uint16_t *utf16_string,
     size_t utf16_string_size,
     libcerror_error_t **error );

LIBFSNTFS_EXTERN \
int libfsntfs_usn_record_get_file_entry(
     libfsntfs_usn_record_t *usn_record,
     libfsntfs_volume_t *volume,
     libfsntfs_file_entry_t **file_entry,
     libcerror_error_t **error );

LIBFSNTFS_EXTERN \
int libfsntfs_usn_record_get_parent_file_entry(
     libfsntfs_usn_record_t *usn_record,
     libfsntfs_volume_t *volume,
     libfsntfs_file_entry_t **parent_file_entry,
     libcerror_error_t **error );

LIBFSNTFS_EXTERN \
int libfsntfs_usn_record_get_utf8_path_size(
     libfsntfs_usn_record_t *usn_record,
     libfsntfs_volume_t *volume,
     size_t *utf8_string_size,
     libcerror_error_t **error );

LIBFSNTFS_EXTERN \
int libfsntfs_usn_record_get_utf8_path(
     libfsntfs_usn_record_t *usn_record,
     libfsntfs_volume_t *volume,
     uint8_t *utf8_string,
     size_t utf8_string_size,
     libcerror_error_t **error );

#if defined( __cplusplus )
}
#endif

#endif /* !defined( _LIBFSNTFS_USN_RECORD_H ) */
