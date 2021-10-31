/* Copyright 2021, Stephen Fryatt (info@stevefryatt.org.uk)
 *
 * This file is part of Strong Extract:
 *
 *   http://www.stevefryatt.org.uk/risc-os/
 *
 * Licensed under the EUPL, Version 1.2 only (the "Licence");
 * You may not use this work except in compliance with the
 * Licence.
 *
 * You may obtain a copy of the Licence at:
 *
 *   http://joinup.ec.europa.eu/software/page/eupl
 *
 * Unless required by applicable law or agreed to in
 * writing, software distributed under the Licence is
 * distributed on an "AS IS" basis, WITHOUT WARRANTIES
 * OR CONDITIONS OF ANY KIND, either express or implied.
 *
 * See the Licence for the specific language governing
 * permissions and limitations under the Licence.
 */

/**
 * \file files.h
 *
 * Platform-Agnostic File and Directory Access Interface.
 */

#ifndef STRONGEX_FILES_H
#define STRONGEX_FILES_H

#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>

/**
 * The default file type applied when not otherwise specified.
 */

#define FILES_TYPE_DEFAULT (0xffd)

/**
 * Don't include a type when building the filename.
 */

#define FILES_TYPE_OMIT (0xffffffffu)

#ifdef LINUX
#define FILES_PATH_SEPARATOR "/"
#endif
#ifdef RISCOS
#define FILES_PATH_SEPARATOR "."
#endif

/**
 * Details of an object on disc.
 */

struct files_object_info {
	char				*name;		/**< The name of the object.			*/
	size_t				size;		/**< The size of the object in bytes.		*/
	uint32_t			filetype;	/**< The RISC OS filetype of the object.	*/
	char				*real_name;	/**< The name of the object stored on disc.	*/

	struct files_object_info	*next;		/**< Pointer to the next object, or NULL.	*/
};

/**
 * Read the contents of a directory, returning a linked list of objects.
 *
 * \param *path		Pointer to the path to the required directory.
 * \return		Pointer to the head of a linked list of objects, or NULL.
 */

struct files_object_info *files_read_directory_contents(char *path);

/**
 * Return object info details for a single directory on disc.
 *
 * \param *path		Pointer to the directory path.
 * \param strict	Should the directory exist.
 * \return		Pointer to the information, or NULL.
 */

struct files_object_info *files_read_directory_info(char *path, bool strict);

/**
 * Make a filename up using its name and filetype, and create a new
 * buffer for it using malloc(). It is up to the caller to release
 * this using free() once no longer required.
 *
 * \param *name		Pointer to the filename.
 * \param filetype	The RISC OS filetype.
 * \return		Pointer to the resulting name buffer.
 */

char *files_make_filename(char *name, uint32_t filetype);

/**
 * Set the RISC OS filetype of a file.
 * \param *name		Pointer to the filename.
 * \param filetype	The RISC OS filetype.
 * \return		True if successful; False on failure.
 */

bool files_set_filetype(char *name, uint32_t filetype);

/**
 * Create a new directory.
 *
 * \param *path		Pointer to the required directory path.
 * \return		True if successful; False on failure.
 */

bool files_make_directory(char *path);

/**
 * Delete an empty directory.
 *
 * \param *path		Pointer to the required directory path.
 * \return		True if successful; False on failure.
 */

bool files_delete_directory(char *path);

/**
 * Write a file to disc.
 *
 * \param *path		Pointer to the required file path.
 * \param *data		Pointer to the data to be written.
 * \param length	The length of the data to be written.
 * \return		True if successful; False on failure.
 */

bool files_write_file(char *path, char *data, size_t length);

/**
 * Delete a file
 *
 * \param *path		Pointer to the required file path.
 * \return		True if successful; False on failure.
 */

bool files_delete_file(char *path);

#endif
