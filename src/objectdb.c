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
 * \file objectdb.c
 *
 * Object Database, implementation.
 */

#include <ctype.h>
#include <stddef.h>
#include <string.h>

/* Local source headers. */

#include "objectdb.h"

#include "msg.h"
#include "string.h"

/* Data Structures */

/**
 * Details of a file within the manual.
 */

struct objectdb_file {
	char				*filename;

	struct objectdb_directory	*parent;
	struct objectdb_file		*next;
};

/**
 * Details of a directory within the manual.
 */

struct objectdb_dir {
	char				*filename;

	struct objectdb_dir		*directories;
	struct objectdb_file		*files;

	struct objectdb_directory	*parent;
	struct objectdb_dir		*next;
};

/* Global Variables. */

/**
 * The root directory in the structure.
 */

struct objectdb_dir *objectdb_root = NULL;

/* Static Function Prototypes. */

static void objectdb_directory_report(struct objectdb_dir *dir);
static char *objectdb_get_dir_path(struct objectdb_dir *dir, size_t *length, char *separator);


/**
 * Add a directory reference from the StrongHelp manual.
 *
 * \param *parent	Pointer to the parent directory, or NULL for the root.
 * \param *name		Pointer to the name of the directory.
 * \return		Pointer to the new directory instance, or NULL.
 */

struct objectdb_dir *objectdb_add_stronghelp_directory(struct objectdb_dir *parent, char *name)
{
	struct objectdb_dir *dir;

	if (parent == NULL && objectdb_root != NULL) {
		msg_report(MSG_TOO_MANY_ROOTS);
		return NULL;
	}

	dir = malloc(sizeof(struct objectdb_dir));
	if (dir == NULL) {
		msg_report(MSG_NO_MEMORY);
		return NULL;
	}

	dir->filename = name;

	dir->directories = NULL;
	dir->files = NULL;
	dir->parent = parent;

	if (parent != NULL) {
		dir->next = parent->directories;
		parent->directories = dir;
	} else {
		dir->next = NULL;
		objectdb_root = dir;
	}

	return dir;
}

/**
 * Add a file reference from the StrongHelp manual.
 *
 * \param *parent	Pointer to the parent directory.
 * \param *name		Pointer to the name of the file.
 * \return		Pointer to the new file instance, or NULL.
 */

struct objectdb_file *objectdb_add_stronghelp_file(struct objectdb_dir *parent, char *name)
{
	struct objectdb_file *file;

	if (parent == NULL) {
		msg_report(MSG_NO_PARENT);
		return NULL;
	}

	file = malloc(sizeof(struct objectdb_file));
	if (file == NULL) {
		msg_report(MSG_NO_MEMORY);
		return NULL;
	}

	file->filename = name;
	file->parent = parent;

	file->next = parent->files;
	parent->files = file;

	return file;
}

/**
 * Write a report of the objects held in the database.
 */

void objectdb_create_report(void)
{
	printf("Creating report...\n");
	objectdb_directory_report(objectdb_root);
}

/**
 * Write a report of a directory and all of the directories and files
 * contained within it.
 *
 * \param *dir		Pointer to the directory on which to report.
 */

static void objectdb_directory_report(struct objectdb_dir *dir)
{
	struct objectdb_dir *next_dir;
	struct objectdb_file *next_file;
	char *name;
	size_t length;

	if (dir == NULL)
		return;

	next_file = dir->files;
	while (next_file != NULL) {
		length = strlen(next_file->filename) + 1;
		name = objectdb_get_dir_path(dir, &length, ".");
		if (name != NULL) {
			string_append(name, next_file->filename, length);
			printf("--> File: %s\n", name);
			free(name);
		} else {
			msg_report(MSG_NO_MEMORY);
		}

		next_file = next_file->next;
	}

	next_dir = dir->directories;
	while (next_dir != NULL) {
		objectdb_directory_report(next_dir);
		next_dir = next_dir->next;
	}
}

/**
 * Build up a full path name from a directory object.
 * 
 * On entry, the length variable should be set to hold the number
 * of characters (including terminator) required after the trailing
 * separator of the final directory name; the allocated buffer will
 * then contain sufficient free space.
 * 
 * The returned buffer is allocated using malloc(), and should be
 * freed with free() after use.
 *
 * \param *dir		Pointer to the object to work from.
 * \param *length	Pointer to a variable to hold the length of the name buffer.
 * \param *separator	Pointer to a separator string.
 * \return		Pointer to a buffer holding the assembled name.
 */

static char *objectdb_get_dir_path(struct objectdb_dir *dir, size_t *length, char *separator)
{
	char *name = NULL;

	*length += strlen(dir->filename) + strlen(separator);

	if (dir->parent != NULL) {
		name = objectdb_get_dir_path(dir->parent, length, separator);
	} else {
		name = malloc(*length);
		*name = '\0';
	}

	string_append(name, dir->filename, *length);
	string_append(name, separator, *length);

	return name;
}
