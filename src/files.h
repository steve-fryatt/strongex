/* Copyright 2021, Stephen Fryatt (info@stevefryatt.org.uk)
 *
 * This file is part of Strong Extract:
 *
 *   http://www.stevefryatt.org.uk/software/
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
 * \return		Pointer to the information, or NULL.
 */

struct files_object_info *files_read_directory_info(char *path);

/**
 * Create a new directory.
 *
 * \param *path		Pointer to the required directory path.
 * \return		True if successful; False on failure.
 */

bool files_make_directory(char *path);

#endif
