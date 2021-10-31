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
 * \file objectdb.h
 *
 * Object Database Interface.
 */

#ifndef STRONGEX_OBJECTDB_H
#define STRONGEX_OBJECTDB_H

#include <stdbool.h>

/**
 * The types of path to return from path queries.
 */

enum objectdb_path_type {
	OBJECTDB_PATH_TYPE_AGNOSTIC,		/**< The generic path to the file.		*/
	OBJECTDB_PATH_TYPE_STRONGHELP,		/**< The StrongHelp path to the file.		*/
	OBJECTDB_PATH_TYPE_DISC			/**< The disc-based path to the file.		*/
};

/**
 * A special file type assigned to directories.
 */

#define OBJECTDB_TYPE_DIRECTORY (0x1000)

/**
 * A special file type assigned to objects which don't exist.
 */

#define OBJECTDB_TYPE_UNKNOWN (0xffff)

/**
 * An object instance reference.
 */

struct objectdb_object;


/**
 * Add a directory reference from the StrongHelp manual.
 *
 * \param *parent	Pointer to the parent directory, or NULL for the root.
 * \param *name		Pointer to the name of the directory.
 * \return		Pointer to the new directory instance, or NULL.
 */

struct objectdb_object *objectdb_add_stronghelp_directory(struct objectdb_object *parent, char *name);

/**
 * Add a file reference from the StrongHelp manual.
 *
 * \param *parent	Pointer to the parent directory.
 * \param *name		Pointer to the name of the file.
 * \param size		The size of the file.
 * \param filetype	The filetype of the file.
 * \param *data		Pointer to the file data.
 * \return		Pointer to the new file instance, or NULL.
 */

struct objectdb_object *objectdb_add_stronghelp_file(struct objectdb_object *parent, char *name, size_t size, uint32_t filetype, char *data);

/**
 * Add a directory reference from the disc manual.
 *
 * \param *parent	Pointer to the parent directory, or NULL for the root.
 * \param *name		Pointer to the name of the directory.
 * \param *real_name	Pointer to the real name of the directory.
 * \return		Pointer to the resulting directory instance, or NULL.
 */

struct objectdb_object *objectdb_add_disc_directory(struct objectdb_object *parent, char *name, char *real_name);

/**
 * Add a file reference from the StrongHelp manual.
 *
 * \param *parent	Pointer to the parent directory.
 * \param *name		Pointer to the name of the file.
 * \param *real_name	Pointer to the real name of the file.
 * \param size		The size of the file.
 * \param filetype	The filetype of the file.
 * \return		Pointer to the new file instance, or NULL.
 */

struct objectdb_object *objectdb_add_disc_file(struct objectdb_object *parent, char *name, char *real_name, size_t size, uint32_t filetype);

/**
 * Check the status of the objects held in the database.
 * 
 * \return		True if successful, false on failure.
 */

bool objectdb_check_status(void);

/**
 * Write a report of the object statuses in the database.
 *
 * \param include_all	Should identical objects be included.
 * \return		True if successful, false on failure.
 */

bool objectdb_output_report(bool include_all);

/**
 * Update the objects in the database.
 * 
 * \return		True if successful, false on failure.
 */

bool objectdb_update(void);

/**
 * Get a file path to an object.
 *
 * The path is allocated using malloc(), and must be freed with free() after use.
 *
 * \param *object 	Pointer to the object of interest.
 * \param type		The type of path to return.
 * \param *separator	The directory separator to use.
 * \return		A pointer to the path, or NULL.
 */

char *objectdb_get_path(struct objectdb_object *object, enum objectdb_path_type type, char *separator);

#endif
