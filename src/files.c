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
 * \file files.c
 *
 * Platform-Agnostic File and Directory Access, implementation.
 */

#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#ifdef LINUX
#include <dirent.h>
#include <sys/stat.h>
#endif
#ifdef RISCOS
#include "oslib/os.h"
#include "oslib/osfile.h"
#include "oslib/osgbpb.h"
#endif

/* Local source headers. */

#include "files.h"

#include "msg.h"
#include "string.h"

/**
 * The size of block allocated to RISC OS OS_GBPB calls.
 */

#define FILES_OSGBPB_SIZE 256

/* Static Function Prototypes. */

static void files_link_object(struct files_object_info **list, struct files_object_info *object);
uint32_t files_get_filetype(char *name);

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

		next->filetype = (S_ISDIR(stat_buffer.st_mode)) ? FILES_TYPE_DIRECTORY : files_get_filetype(next->name);

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
		printf("Looping in...\n");
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

		next->filetype = (osgbpb_list->info[0].obj_type == osfile_IS_DIR) ? FILES_TYPE_DIRECTORY : ((osgbpb_list->info[0].load_addr >> 8) & 0xfffu);
		next->size = osgbpb_list->info[0].size;

		files_link_object(&list, next);

		printf("Looping out...\n");
	} while (context != -1);
#endif

	printf("Done!\n");

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
	struct files_object_info *next = NULL;

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

uint32_t files_get_filetype(char *name)
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