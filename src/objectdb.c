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
#include <stdint.h>
#include <stdio.h>
#include <string.h>

/* Local source headers. */

#include "objectdb.h"

#include "files.h"
#include "msg.h"
#include "string.h"

/**
 * The status of an object.
 */

enum objectdb_status {
	OBJECTDB_STATUS_UNKNOWN,
	OBJECTDB_STATUS_IDENTICAL,
	OBJECTDB_STATUS_ADDED,
	OBJECTDB_STATUS_DELETED,
	OBJECTDB_STATUS_TYPE_CHANGED,
	OBJECTDB_STATUS_SIZE_CHANGED,
	OBJECTDB_STATUS_CONTENT_CHANGED,
};

/* Data Structures */

struct objectdb_details {
	char 		*name;
	size_t		size;
	uint32_t	filetype;
	char		*data;
};

/**
 * Details of an object within the manual.
 */

struct objectdb_object {
	char				*name;
	enum objectdb_status		status;

	struct objectdb_details		stronghelp;
	struct objectdb_details		disc;

	struct objectdb_object		*directories;
	struct objectdb_object		*files;

	struct objectdb_object		*parent;
	struct objectdb_object		*next;
};

/* Global Variables. */

/**
 * The root directory in the structure.
 */

struct objectdb_object *objectdb_root = NULL;

/* Static Function Prototypes. */

static void objectdb_link_object(struct objectdb_object **list, struct objectdb_object *object);
static struct objectdb_object *objectdb_find_object(struct objectdb_object *list, char *name);
static void objectdb_check_directory_status(struct objectdb_object *dir);
static bool objectdb_compare_files(struct objectdb_object *object);
static void objectdb_output_directory_report(struct objectdb_object *dir, bool include_all);
static void objectdb_update_directory(struct objectdb_object *dir);
static char *objectdb_get_dir_path(struct objectdb_object *dir, size_t *length, enum objectdb_path_type type, char *separator);

/**
 * Add a directory reference from the StrongHelp manual.
 *
 * \param *parent	Pointer to the parent directory, or NULL for the root.
 * \param *name		Pointer to the name of the directory.
 * \return		Pointer to the new directory instance, or NULL.
 */

struct objectdb_object *objectdb_add_stronghelp_directory(struct objectdb_object *parent, char *name)
{
	struct objectdb_object *dir;

	if (parent == NULL && objectdb_root != NULL) {
		msg_report(MSG_TOO_MANY_ROOTS);
		return NULL;
	}

	dir = malloc(sizeof(struct objectdb_object));
	if (dir == NULL) {
		msg_report(MSG_NO_MEMORY);
		return NULL;
	}

	dir->name = name;
	dir->status = OBJECTDB_STATUS_UNKNOWN;

	dir->stronghelp.name = name;
	dir->stronghelp.size = 0;
	dir->stronghelp.filetype = OBJECTDB_TYPE_DIRECTORY;
	dir->stronghelp.data = NULL;

	dir->disc.name = NULL;
	dir->disc.size = 0;
	dir->disc.filetype = OBJECTDB_TYPE_UNKNOWN;
	dir->disc.data = NULL;

	dir->directories = NULL;
	dir->files = NULL;
	dir->parent = parent;

	if (parent != NULL) {
		objectdb_link_object(&(parent->directories), dir);
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
 * \param size		The size of the file.
 * \param filetype	The filetype of the file.
 * \param *data		Pointer to the file data.
 * \return		Pointer to the new file instance, or NULL.
 */

struct objectdb_object *objectdb_add_stronghelp_file(struct objectdb_object *parent, char *name, size_t size, uint32_t filetype, char *data)
{
	struct objectdb_object *file;

	if (parent == NULL) {
		msg_report(MSG_NO_PARENT);
		return NULL;
	}

	file = malloc(sizeof(struct objectdb_object));
	if (file == NULL) {
		msg_report(MSG_NO_MEMORY);
		return NULL;
	}

	file->name = name;
	file->status = OBJECTDB_STATUS_UNKNOWN;

	file->stronghelp.name = name;
	file->stronghelp.size = size;
	file->stronghelp.filetype = filetype;
	file->stronghelp.data = data;

	file->disc.name = NULL;
	file->disc.size = 0;
	file->disc.filetype = OBJECTDB_TYPE_UNKNOWN;
	file->disc.data = NULL;

	file->directories = NULL;
	file->files = NULL;
	file->parent = parent;

	objectdb_link_object(&(parent->files), file);

	return file;
}

/**
 * Add a directory reference from the disc manual.
 *
 * \param *parent	Pointer to the parent directory, or NULL for the root.
 * \param *name		Pointer to the name of the directory.
 * \param *real_name	Pointer to the real name of the directory.
 * \return		Pointer to the resulting directory instance, or NULL.
 */

struct objectdb_object *objectdb_add_disc_directory(struct objectdb_object *parent, char *name, char *real_name)
{
	struct objectdb_object *dir;

	if (parent == NULL && objectdb_root == NULL) {
		msg_report(MSG_NO_ROOT);
		return NULL;
	}

	dir = (parent == NULL) ? objectdb_root : objectdb_find_object(parent->directories, name);

	if (dir == NULL) {
		printf("No match for directory %s, creating new...\n", name);

		dir = malloc(sizeof(struct objectdb_object));
		if (dir == NULL) {
			msg_report(MSG_NO_MEMORY);
			return NULL;
		}

		dir->name = name;
		dir->status = OBJECTDB_STATUS_UNKNOWN;

		dir->stronghelp.name = NULL;
		dir->stronghelp.size = 0;
		dir->stronghelp.filetype = OBJECTDB_TYPE_UNKNOWN;
		dir->stronghelp.data = NULL;

		dir->directories = NULL;
		dir->files = NULL;
		dir->parent = parent;

		if (parent != NULL)
			objectdb_link_object(&(parent->directories), dir);
	} else {
		printf("Found match for directory %s... parent=0x%x, shname=%s\n", name, dir->parent, dir->stronghelp.name);
	}

	dir->disc.name = real_name;
	dir->disc.size = 0;
	dir->disc.filetype = OBJECTDB_TYPE_DIRECTORY;
	dir->disc.data = NULL;

	return dir;
}

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

struct objectdb_object *objectdb_add_disc_file(struct objectdb_object *parent, char *name, char *real_name, size_t size, uint32_t filetype)
{
	struct objectdb_object *file;

	if (parent == NULL) {
		msg_report(MSG_NO_PARENT);
		return NULL;
	}

	file = objectdb_find_object(parent->files, name);

	if (file == NULL) {
		printf("No match for file %s, creating new...\n", name);

		file = malloc(sizeof(struct objectdb_object));
		if (file == NULL) {
			msg_report(MSG_NO_MEMORY);
			return NULL;
		}

		file->name = name;
		file->status = OBJECTDB_STATUS_UNKNOWN;

		file->stronghelp.name = NULL;
		file->stronghelp.size = 0;
		file->stronghelp.filetype = OBJECTDB_TYPE_UNKNOWN;
		file->stronghelp.data = NULL;

		file->directories = NULL;
		file->files = NULL;
		file->parent = parent;

		objectdb_link_object(&(parent->files), file);
	} else {
		printf("Found match for file %s... parent=0x%x\n", name, file->parent);
	}

	file->disc.name = real_name;
	file->disc.size = size;
	file->disc.filetype = filetype;
	file->disc.data = NULL;

	return file;
}

/**
 * Find an object by matching the common filename.
 *
 * \param *list		Pointer to the first object in the list to search.
 * \param *name		Pointer to the name to be matched.
 * \return		Pointer to the matched object, or NULL.
 */

static struct objectdb_object *objectdb_find_object(struct objectdb_object *list, char *name)
{
	int result = -1;

	while (list != NULL && ((list->name == NULL) || ((result = strcmp(list->name, name)) < 0)))
		list = list->next;

	return (result == 0) ? list : NULL;
}

/**
 * Link a new object into an object list, in the correct position alphabetically.
 *
 * \param **list	Pointer to the list head pointer location.
 * \param *object	Pointer to the new object to link.
 */

static void objectdb_link_object(struct objectdb_object **list, struct objectdb_object *object)
{
	if (list == NULL || object == NULL)
		return;

	while (*list != NULL && strcmp((*list)->name, object->name) < 0)
		list = &((*list)->next);

	object->next = *list;
	*list = object;
}

/**
 * Check the status of the objects held in the database.
 */

void objectdb_check_status(void)
{
	objectdb_check_directory_status(objectdb_root);
}

/**
 * Check the statis of the objects held in a directory, and in all of the
 * directories and files contained within it.
 *
 * \param *dir		Pointer to the directory to be checked.
 */

static void objectdb_check_directory_status(struct objectdb_object *dir)
{
	struct objectdb_object *object;
	char *name;

	if (dir == NULL)
		return;

	if (dir->stronghelp.name == NULL && dir->disc.name != NULL)
		dir->status = OBJECTDB_STATUS_DELETED;
	else if (dir->stronghelp.name != NULL && dir->disc.name == NULL)
		dir->status = OBJECTDB_STATUS_ADDED;
	else
		dir->status = OBJECTDB_STATUS_IDENTICAL;

	object = dir->files;
	while (object != NULL) {
		if (object->stronghelp.name == NULL && object->disc.name != NULL)
			object->status = OBJECTDB_STATUS_DELETED;
		else if (object->stronghelp.name != NULL && object->disc.name == NULL)
			object->status = OBJECTDB_STATUS_ADDED;
		else if (object->stronghelp.filetype != object->disc.filetype)
			object->status = OBJECTDB_STATUS_TYPE_CHANGED;
		else if (object->stronghelp.size != object->disc.size)
			object->status = OBJECTDB_STATUS_SIZE_CHANGED;
		else if (!objectdb_compare_files(object))
			object->status = OBJECTDB_STATUS_CONTENT_CHANGED;
		else
			object->status = OBJECTDB_STATUS_IDENTICAL;

		object = object->next;
	}

	object = dir->directories;
	while (object != NULL) {
		objectdb_check_directory_status(object);
		object = object->next;
	}
}

/**
 * Perform a byte-wise content comparison between the StrongHelp and disc-based
 * versions of a file.
 *
 * \param *object	Pointer to the file object to be tested.
 * \return		True if the files are identical, False if different.
 */

static bool objectdb_compare_files(struct objectdb_object *object)
{
	char *filename;
	FILE *file = NULL;
	bool identical = true;
	int i = 0;

	if (object == NULL || object->stronghelp.data == NULL || object->disc.name == NULL)
		return false;

	filename = objectdb_get_path(object, OBJECTDB_PATH_TYPE_DISC, FILES_PATH_SEPARATOR);
	if (filename == NULL)
		return false;

	file = fopen(filename, "rw");
	if (file == NULL)
		return;

	while (i < object->stronghelp.size && !feof(file) && identical) {
		if (object->stronghelp.data[i++] != (char) fgetc(file))
			identical = false;
	}

	fclose(file);

	return identical;
}

/**
 * Write a report of the object statuses in the database.
 *
 * \param include_all	Should identical objects be included.
 */

void objectdb_output_report(bool include_all)
{
	objectdb_output_directory_report(objectdb_root, include_all);
}

/**
 * Write a report of the object statuses in a directory, and all subdirectories
 * below that.
 *
 * \param *dir		Pointer to the directory on which to report.
 * \param include_all	Should identical objects be included.
 */

static void objectdb_output_directory_report(struct objectdb_object *dir, bool include_all)
{
	struct objectdb_object *object;
	char *name;

	if (dir == NULL)
		return;

	name = objectdb_get_path(dir, OBJECTDB_PATH_TYPE_AGNOSTIC, ".");
	if (name != NULL) {
		switch (dir->status) {
		case OBJECTDB_STATUS_ADDED:
			printf("Directory %s Added\n", name);
			break;
		case OBJECTDB_STATUS_DELETED:
			printf("Directory %s Deleted\n", name);
			break;
		case OBJECTDB_STATUS_IDENTICAL:
			if (include_all)
				printf("Directory %s Unchanged\n", name);
			break;
		default:
			msg_report(MSG_BAD_STATUS, name);
			break;
		}

		free(name);
	} else {
		msg_report(MSG_NO_MEMORY);
	}

	object = dir->files;
	while (object != NULL) {
		name = objectdb_get_path(object, OBJECTDB_PATH_TYPE_AGNOSTIC, ".");
		if (name != NULL) {
			switch (object->status) {
			case OBJECTDB_STATUS_ADDED:
				printf("File %s Added\n", name);
				break;
			case OBJECTDB_STATUS_DELETED:
				printf("File %s Deleted\n", name);
				break;
			case OBJECTDB_STATUS_TYPE_CHANGED:
				printf("File %s Changed Type\n", name);
				break;
			case OBJECTDB_STATUS_SIZE_CHANGED:
			case OBJECTDB_STATUS_CONTENT_CHANGED:
				printf("File %s Changed Content\n", name);
				break;
			case OBJECTDB_STATUS_IDENTICAL:
				if (include_all)
					printf("File %s Unchanged\n", name);
				break;
			default:
				msg_report(MSG_BAD_STATUS, name);
				break;
			}

			free(name);
		} else {
			msg_report(MSG_NO_MEMORY);
		}

		object = object->next;
	}

	object = dir->directories;
	while (object != NULL) {
		objectdb_output_directory_report(object, include_all);
		object = object->next;
	}
}

/**
 * Update the objects in the database.
 */

void objectdb_output(void)
{
	printf("Running update...\n");
	objectdb_update_directory(objectdb_root);
}

static char *objectdb_get_filename(char *name, uint32_t filetype)
{
	size_t length = 0;
	char *buffer = NULL;

	length = strlen(name) + 5;
	buffer = malloc(length);
	if (buffer == NULL)
		return NULL;

	snprintf(buffer, length, "%s,%3x", name, filetype);
	buffer[length - 1] = '\0';

	return buffer;
}

/**
 * Update a given output directory and all of the folders below it.
 *
 * \param *dir		Pointer to the directory directory to be output.
 */

static void objectdb_update_directory(struct objectdb_object *dir)
{
	struct objectdb_object *object;
	FILE *file = NULL;
	char *path = NULL;

	if (dir == NULL)
		return;

	switch (dir->status) {
	case OBJECTDB_STATUS_ADDED:
		if (dir->disc.name == NULL)
			dir->disc.name = dir->name;

		path = objectdb_get_path(dir, OBJECTDB_PATH_TYPE_DISC, FILES_PATH_SEPARATOR);
		files_make_directory(path);
		free(path);
		break;
	case OBJECTDB_STATUS_DELETED:
		// Delete directory.
		break;
	default:
		break;
	}

	object = dir->files;
	while (object != NULL) {
		switch (object->status) {
		case OBJECTDB_STATUS_ADDED:
			object->disc.name = objectdb_get_filename(object->stronghelp.name, object->stronghelp.filetype);

			path = objectdb_get_path(object, OBJECTDB_PATH_TYPE_DISC, FILES_PATH_SEPARATOR);
			file = fopen(path, "w");
			if (file != NULL) {
				fwrite(object->stronghelp.data, object->stronghelp.size, sizeof(char), file);
				fclose(file);
			}
			free(path);
			break;
		case OBJECTDB_STATUS_DELETED:
			break;
		case OBJECTDB_STATUS_TYPE_CHANGED:
		case OBJECTDB_STATUS_SIZE_CHANGED:
		case OBJECTDB_STATUS_CONTENT_CHANGED:
			break;
		default:
			break;
		}

		object = object->next;
	}

	object = dir->directories;
	while (object != NULL) {
		objectdb_update_directory(object);
		object = object->next;
	}
}

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

char *objectdb_get_path(struct objectdb_object *object, enum objectdb_path_type type, char *separator)
{
	size_t length = 0;

	return objectdb_get_dir_path(object, &length, type, separator);
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
 * \param type		The type of path to generate.
 * \param *separator	Pointer to a separator string.
 * \return		Pointer to a buffer holding the assembled name.
 */

static char *objectdb_get_dir_path(struct objectdb_object *dir, size_t *length, enum objectdb_path_type type, char *separator)
{
	char *name = NULL, *part = NULL;

	switch (type) {
	case OBJECTDB_PATH_TYPE_AGNOSTIC:
		part = dir->name;
		break;
	case OBJECTDB_PATH_TYPE_STRONGHELP:
		part = dir->stronghelp.name;
		break;
	case OBJECTDB_PATH_TYPE_DISC:
		part = dir->disc.name;
		break;
	}

	*length += strlen(part) + strlen(separator);

	if (dir->parent != NULL) {
		name = objectdb_get_dir_path(dir->parent, length, type, separator);
		string_append(name, separator, *length);
	} else {
		name = malloc(*length);
		*name = '\0';
	}

	string_append(name, part, *length);

	return name;
}
