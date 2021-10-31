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
 * \file files.c
 *
 * Platform-Agnostic File and Directory Access, implementation.
 */

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef LINUX
#include <dirent.h>
#include <sys/stat.h>
#include <unistd.h>
#endif
#ifdef RISCOS
#include "oslib/os.h"
#include "oslib/osfile.h"
#include "oslib/osgbpb.h"
#endif

/* Local source headers. */

#include "files.h"

#include "msg.h"
#include "objectdb.h"
#include "string.h"

/**
 * The size of block allocated to RISC OS OS_GBPB calls.
 */

#define FILES_OSGBPB_SIZE 256

/* Static Function Prototypes. */

static void files_link_object(struct files_object_info **list, struct files_object_info *object);
#ifdef LINUX
static uint32_t files_get_filetype(char *name);
#endif
static char *files_convert_name_to_riscos(char *name);

/**
 * Read the contents of a directory, returning a linked list of objects.
 *
 * \param *path		Pointer to the path to the required directory.
 * \return		Pointer to the head of a linked list of objects, or NULL.
 */

struct files_object_info *files_read_directory_contents(char *path)
{
	struct files_object_info *list = NULL, *next = NULL;

#ifdef LINUX
	DIR *directory = NULL;
	struct dirent *entry = NULL;
	struct stat stat_buffer;
	int directory_fd;
	size_t length;

	/* Open a directory stream. */

	directory = opendir(path);
	if (directory == NULL) {
		msg_report(MSG_DIR_READ_FAIL);
		return NULL;
	}

	directory_fd = dirfd(directory);

	while ((entry = readdir(directory)) != NULL) {
		if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0)
			continue;

		/* Stat the file for more details. */

		if (fstatat(directory_fd, entry->d_name, &stat_buffer, 0) != 0) {
			msg_report(MSG_DIR_READ_FAIL);
			break;
		}

		/* Allocate memory for the file details. */

		length = sizeof(struct files_object_info) + (2 * (strlen(entry->d_name) + 1));

		next = malloc(length);
		if (next == NULL) {
			msg_report(MSG_NO_MEMORY);
			break;
		}

		/* Store the filenames. */

		next->name = (char *) (next + 1);
		string_copy(next->name, entry->d_name, strlen(entry->d_name) + 1);

		next->real_name = next->name + strlen(entry->d_name) + 1;
		string_copy(next->real_name, entry->d_name, strlen(entry->d_name) + 1);

		/* Work out the filetype details. */

		next->filetype = (S_ISDIR(stat_buffer.st_mode)) ? OBJECTDB_TYPE_DIRECTORY : files_get_filetype(next->name);

		/* Fix the filename conventions. */

		files_convert_name_to_riscos(next->name);

		/* Store the file details. */

		next->size = stat_buffer.st_size;

		files_link_object(&list, next);
	}

	closedir(directory);
#endif
#ifdef RISCOS
	os_error *error;
	int8_t buffer[FILES_OSGBPB_SIZE];
	int context = 0, read = 0;
	osgbpb_info_list *osgbpb_list = (osgbpb_info_list *) buffer;
	size_t length = 0;

	do {
		error = xosgbpb_dir_entries_info(path, osgbpb_list, 1, context, FILES_OSGBPB_SIZE, "*", &read, &context);
		if (error != NULL) {
			msg_report(MSG_DIR_READ_FAIL);
			break;
		}

		if (read == 0)
			continue;

		/* Allocate memory for the file details. */

		length = (sizeof(struct files_object_info) + strlen(osgbpb_list->info[0].name) + 1);

		next = malloc(length);
		if (next == NULL) {
			msg_report(MSG_NO_MEMORY);
			break;
		}

		/* Store the file details. */

		next->name = (char *) (next + 1);
		string_copy(next->name, osgbpb_list->info[0].name, strlen(osgbpb_list->info[0].name) + 1);

		next->real_name = next->name;

		next->filetype = (osgbpb_list->info[0].obj_type == osfile_IS_DIR) ? OBJECTDB_TYPE_DIRECTORY : ((osgbpb_list->info[0].load_addr >> 8) & 0xfffu);
		next->size = osgbpb_list->info[0].size;

		files_link_object(&list, next);
	} while (context != -1);
#endif

	return list;
}

/**
 * Link a new object into the object list, in the correct position alphabetically.
 *
 * \param **list	Pointer to the list head pointer location.
 * \param *object	Pointer to the new object to link.
 */

static void files_link_object(struct files_object_info **list, struct files_object_info *object)
{
	if (list == NULL || object == NULL)
		return;

	while (*list != NULL && strcmp((*list)->name, object->name) < 0)
		list = &((*list)->next);

	object->next = *list;
	*list = object;
}

/**
 * Read the type of a file from an ,xxx extension to its name, and then trim
 * that extension off the supplied buffer.
 *
 * \param *name		Pointer to the name buffer.
 * \return		The filetype identified from the name.
 */

#ifdef LINUX
static uint32_t files_get_filetype(char *name)
{
	size_t length = 0;
	long int value;

	if (name == NULL)
		return FILES_TYPE_DEFAULT;

	/* If the name isn't long enough for a ,xxx extension, return default. */

	length = strlen(name);
	if (length < 4 || name[length - 4] != ',')
		return FILES_TYPE_DEFAULT;

	/* Attempt to convert the value, and return default on failure. */

	value = strtol(name + length - 3, NULL, 16);

	if (value < 0 || value > 0xfff)
		return FILES_TYPE_DEFAULT;

	/* If successful, trim the type off the filename. */

	name[length - 4] = '\0';

	return (uint32_t) value;
}
#endif

/**
 * Convert a filename from native format into RISC OS.
 *
 * \param *name		Pointer to the filename to convert.
 * \return		Pointer to the converted name.
 */

static char *files_convert_name_to_riscos(char *name)
{
#ifdef LINUX
	char *c = name;

	while (c != NULL && *c != '\0') {
		if (*c == '.')
			*c = '/';
		c++;
	}
#endif

	return name;
}

/**
 * Make a filename up using its name and filetype, and create a new
 * buffer for it using malloc(). It is up to the caller to release
 * this using free() once no longer required.
 *
 * \param *name		Pointer to the filename.
 * \param filetype	The RISC OS filetype.
 * \return		Pointer to the resulting name buffer.
 */

char *files_make_filename(char *name, uint32_t filetype)
{
	size_t length = 0;
	char *buffer = NULL;

#ifdef RISCOS
	filetype = FILES_TYPE_OMIT;
#endif

	length = strlen(name) + ((filetype == FILES_TYPE_OMIT) ? 1 : 5);

	buffer = malloc(length);
	if (buffer == NULL)
		return NULL;

	if (filetype == FILES_TYPE_OMIT || filetype == OBJECTDB_TYPE_DIRECTORY)
		snprintf(buffer, length, "%s", name);
	else
		snprintf(buffer, length, "%s,%3x", name, filetype);

	buffer[length - 1] = '\0';

#ifdef LINUX
	{
		char *c = buffer;

		while (*c != '\0') {
			if (*c == '/')
				*c = '.';
			c++;
		}
	}
#endif

	return buffer;
}

/**
 * Set the RISC OS filetype of a file.
 * \param *path		Pointer to the filename.
 * \param filetype	The RISC OS filetype.
 * \return		True if successful; False on failure.
 */

bool files_set_filetype(char *path, uint32_t filetype)
{
#ifdef RISCOS
	if (xosfile_set_type(path, filetype) != NULL)
		return false;
#endif

	return true;
}

/**
 * Return object info details for a single directory on disc.
 * 
 * If strict is applied, the directory must exist on disc for an
 * object to be returned. Othertwise, so long as there is not a
 * non-directory object in the location, a phantom object will
 * be returned.
 *
 * \param *path		Pointer to the directory path.
 * \param strict	Should the directory exist.
 * \return		Pointer to the information, or NULL.
 */

struct files_object_info *files_read_directory_info(char *path, bool strict)
{
	struct files_object_info *info;
	size_t length;

#ifdef LINUX
	int result;
	struct stat stat_buffer;

	result = stat(path, &stat_buffer);

	if (((result != 0) && strict) || ((result == 0) && !strict && !S_ISDIR(stat_buffer.st_mode))) {
		if ((result == 0) && !S_ISDIR(stat_buffer.st_mode))
			msg_report(MSG_NOT_DIR, path);
		return NULL;
	}
#endif
#ifdef RISCOS
	fileswitch_object_type type;

	if (xosfile_read_no_path(path, &type, NULL, NULL, NULL, NULL) != NULL) {
		msg_report(MSG_DIR_READ_FAIL);
		return NULL;
	}

	if (type != fileswitch_IS_DIR && (type != fileswitch_NOT_FOUND || strict)) {
		if (type != fileswitch_NOT_FOUND)
			msg_report(MSG_NOT_DIR, path);
		return NULL;
	}
#endif

	length = sizeof(struct files_object_info) + strlen(path) + 1;

	info = malloc(length);
	if (info == NULL) {
		msg_report(MSG_NO_MEMORY);
		return NULL;
	}

	/* Store the filenames. */

	info->name = (char *) (info + 1);
	string_copy(info->name, path, strlen(path) + 1);

	info->real_name = info->name;
	info->size = 0;
	info->filetype = OBJECTDB_TYPE_DIRECTORY;
	info->next = NULL;

	files_convert_name_to_riscos(info->name);

	return info;
}

/**
 * Create a new directory.
 *
 * \param *path		Pointer to the required directory path.
 * \return		True if successful; False on failure.
 */

bool files_make_directory(char *path)
{
#ifdef LINUX
	if (mkdir(path, 0775) != 0)
		return false;
#endif
#ifdef RISCOS
	if (xosfile_create_dir(path, 0) != NULL)
		return false;
#endif

	return true;
}

/**
 * Delete an empty directory.
 *
 * \param *path		Pointer to the required directory path.
 * \return		True if successful; False on failure.
 */

bool files_delete_directory(char *path)
{
#ifdef LINUX
	if (rmdir(path) != 0)
		return false;
#endif
#ifdef RISCOS
	if (xosfile_delete(path, NULL, NULL, NULL, NULL, NULL) != NULL)
		return false;
#endif

	return true;
}

/**
 * Write a file to disc.
 *
 * \param *path		Pointer to the required file path.
 * \param *data		Pointer to the data to be written.
 * \param length	The length of the data to be written.
 * \return		True if successful; False on failure.
 */

bool files_write_file(char *path, char *data, size_t length)
{
	FILE *file;
	size_t to_write = length;
	size_t written;

	file = fopen(path, "w");
	if (file == NULL)
		return false;

	while (to_write > 0) {
		written = fwrite(data, sizeof(char), length, file);
		if (written == 0)
			break;

		to_write -= written;
	}

	fclose(file);

	return (to_write > 0) ? false : true;
}

/**
 * Delete a file
 *
 * \param *path		Pointer to the required file path.
 * \return		True if successful; False on failure.
 */

bool files_delete_file(char *path)
{
#ifdef LINUX
	if (unlink(path) != 0)
		return false;
#endif
#ifdef RISCOS
	if (xosfile_delete(path, NULL, NULL, NULL, NULL, NULL) != NULL)
		return false;
#endif

	return true;
}
