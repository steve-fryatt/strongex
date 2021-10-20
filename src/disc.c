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
 * \file disc.c
 *
 * Disc File Utilities, implementation.
 */

#include <stdio.h>
#include <stdlib.h>

/* Local source headers. */

#include "disc.h"

#include "files.h"
#include "msg.h"
#include "objectdb.h"

/* Static Function Prototypes. */

static void disc_process_object(struct files_object_info *entry, struct objectdb_object *parent);
static void disc_process_directory_entries(struct objectdb_object *object);

/* Initialise a folder on disc, and roughly validate its
 * contents.
 *
 * \param *file		Pointer to the folder path.
 */

void disc_initialise_folder(char *path)
{
	struct files_object_info *root;

	/* Validate the directory entries. */

	root = files_read_directory_info(path);
	if (root == NULL)
		return;

	disc_process_object(root, NULL);
}

/**
 * Process an object.
 *
 * \param *entry	Pointer to the directory entry for the object.
 * \param *parent	Pointer to the Object DB entry for the parent, or NULL.
 */

static void disc_process_object(struct files_object_info *entry, struct objectdb_object *parent)
{
	struct objectdb_object *object = NULL;

	if (entry == NULL)
		return;

	/* Create an Object DB entry for the directory. */

	if (entry->filetype == OBJECTDB_TYPE_DIRECTORY) {
		object = objectdb_add_disc_directory(parent, entry->name, entry->real_name);

		printf("Directory object... name=%s, size=%d, real name=%s\n", entry->name, entry->size, entry->real_name);

		disc_process_directory_entries(object);

		printf("...Directory done %s\n", entry->name);
	} else if (entry->filetype != OBJECTDB_TYPE_UNKNOWN) {
		objectdb_add_disc_file(parent, entry->name, entry->real_name, entry->size, entry->filetype);
		printf("File object... name=%s, filetype=0x%x, size=%d, real name=%s\n", entry->name, entry->filetype, entry->size, entry->real_name);
	} else {
		msg_report(MSG_BAD_FILETYPE, entry->filetype);
	}
}

/**
 * Process a block of directory entries, recursing down into any
 * subdirectories.
 *
 * \param *object	Pointer to the Object DB entry for the directory.
 */

static void disc_process_directory_entries(struct objectdb_object *object)
{
	struct files_object_info *entries;
	char *path;

	/* Validate the offset and length. */

	if (object == NULL)
		return;

	/* Read the directory on disc. */

	path = objectdb_get_path(object, OBJECTDB_PATH_TYPE_DISC, FILES_PATH_SEPARATOR);

	entries = files_read_directory_contents(path);

	/* Process the entries. */

	while (entries != NULL) {
		disc_process_object(entries, object);

		entries = entries->next;
	}
}
