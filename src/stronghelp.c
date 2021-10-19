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
 * \file stronghelp.c
 *
 * StrongHelp File Utilities, implementation.
 */

#include <ctype.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

/* Local source headers. */

#include "stronghelp.h"

#include "msg.h"
#include "objectdb.h"

/* Magic Words used in file blocks. */

#define STRONGHELP_FILE_WORD (0x504c4548)
#define STRONGHELP_DIR_WORD (0x24524944)
#define STRONGHELP_DATA_WORD (0x41544144)
#define STRONGHELP_FREE_WORD (0x45455246)

/**
 * A StrongHelp file root block.
 */

struct stronghelp_file_root {
	int32_t		help;
	int32_t		size;
	int32_t		version;
	int32_t		free_offset;
};

/**
 * A StrongHelp directory entry block.
 */

struct stronghelp_file_dir_entry {
	int32_t		object_offset;
	int32_t		load_address;
	int32_t		exec_address;
	int32_t		size;
	int32_t		flags;
	int32_t		reserved;
	char		filename[4];
	/* The filename follows this block, zero terminated and padded to a word boundary. */
};

/**
 * A StrongHelp directory block header.
 */

struct stronghelp_file_dir_block {
	int32_t		dir;
	int32_t		size;
	int32_t		used;
};

/**
 * A StrongHelp data (file) block header.
 */

struct stronghelp_file_data_block {
	int32_t		data;
	int32_t		size;
};

/**
 * A StrongHelp free block header.
 */

struct stronghelp_file_free_block {
	int32_t		free;
	int32_t		free_size;
	int32_t		next_offset;
}; 

/* Global Variables */

/**
 * Pointer to the root of the StrongHelp manual.
 */
static int8_t *stronghelp_file_root = NULL;

/**
 * The length of the StrongHelp manual.
 */
static int32_t stronghelp_file_length = 0;

/* Static Function Prototypes */

static void stronghelp_process_object(struct stronghelp_file_dir_entry *entry, struct objectdb_object *parent);
static void stronghelp_process_directory_entries(int32_t offset, size_t length, struct objectdb_object *object);

static int32_t stronghelp_walk_free_space(int32_t offset);
static void *stronghelp_get_block_address(int32_t offset, size_t min_size);


/* Initialise a StrongHelp file and roughly validate its
 * contents.
 *
 * \param *file		Pointer to the file in memory.
 * \param length	The length of the file.
 */

void stronghelp_initialise_file(int8_t *file, size_t length)
{
	struct stronghelp_file_root *header;
	struct stronghelp_file_dir_entry *root;
	int32_t free_space = 0;

	stronghelp_file_root = file;
	stronghelp_file_length = length;

	printf("Accepted file at 0x%x of %d bytes\n", file, length);

	/* Validate the file header. */

	header = stronghelp_get_block_address(0, sizeof(struct stronghelp_file_root));

	printf("Magic Word: 0x%x\n", header->help);
	printf("Version: %d\n", header->version);
	printf("Size: %d\n", header->size);
	printf("Free Space offset: %d\n", header->free_offset);

	if (header->help != STRONGHELP_FILE_WORD) {
		msg_report(MSG_BAD_FILE_MAGIC, header->help);
		return;
	}	

	/* Validate the free space list. */

	free_space = stronghelp_walk_free_space(header->free_offset);

	printf ("Total Free Space: %d bytes\n", free_space);

	/* Validate the directory entries. */

	root = stronghelp_get_block_address(16, sizeof(struct stronghelp_file_dir_entry));
	if (root == NULL) {
		msg_report(MSG_MISSING_ROOT);
		return;
	}

	stronghelp_process_object(root, NULL);
}

/**
 * Process an object.
 *
 * \param *entry	Pointer to the directory entry for the object.
 * \param *parent	Pointer to the Object DB entry for the parent, or NULL.
 */

static void stronghelp_process_object(struct stronghelp_file_dir_entry *entry, struct objectdb_object *parent)
{
	struct stronghelp_file_data_block *data;
	struct stronghelp_file_dir_block *dir;
	size_t path_length = 0, path_written = 0;
	struct objectdb_object *object = NULL;

	if (entry == NULL)
		return;

	/* Start by assuming that the object is a file, since that has a smaller header. */

	data = stronghelp_get_block_address(entry->object_offset, sizeof(struct stronghelp_file_data_block));
	if (data == NULL)
		return;

	/* Create an Object DB entry for the directory. */

	if (data->data == STRONGHELP_DATA_WORD) {
		objectdb_add_stronghelp_file(parent, entry->filename);
		printf("File object... name=%s, size=%d.\n", entry->filename, data->size);
	} else if (data->data == STRONGHELP_DIR_WORD) {
		object = objectdb_add_stronghelp_directory(parent, entry->filename);

		dir = (struct stronghelp_file_dir_block *) data;

		printf("Directory object... name=%s, size=%d, used=%d\n", entry->filename, dir->size, dir->used);

		stronghelp_process_directory_entries(entry->object_offset + sizeof(struct stronghelp_file_dir_block),
				dir->used - sizeof(struct stronghelp_file_dir_block), object);

		printf("...Directory done\n");
	} else {
		msg_report(MSG_BAD_OBJECT_MAGIC, data->data);
	}
}

/**
 * Process a block of directory entries, recursing down into any
 * subdirectories.
 *
 * \param offset	The file offset of the first entry.
 * \param length	The length of the data in the block.
 * \param *object	Pointer to the Object DB entry for the directory.
 */

static void stronghelp_process_directory_entries(int32_t offset, size_t length, struct objectdb_object *object)
{
	struct stronghelp_file_dir_entry *entry;
	int32_t end;

	/* Validate the offset and length. */

	if (offset < 0) {
		msg_report(MSG_BAD_OFFSET, offset);
		return;
	}

	if (length < 0) {
		msg_report(MSG_BAD_SIZE, length);
		return;
	}

	/* Find the offset of the end of the block in the file. */

	end = offset + length;

	if (end >= stronghelp_file_length) {
		msg_report(MSG_OFFSET_RANGE, offset, length, stronghelp_file_length);
		return;
	}

	/* Process the entries. */

	while (offset < end) {
		entry = stronghelp_get_block_address(offset, sizeof(struct stronghelp_file_dir_entry));
		if (entry == NULL) {
			msg_report(MSG_BAD_DIR_ENTRY);
			break;
		}

		stronghelp_process_object(entry, object);

		/* The struct is padded to 4 bytes, so there's no need to add 3 to this. */

		offset += ((int) (sizeof(struct stronghelp_file_dir_entry) + strlen(entry->filename))) & ~3;
	}
}

/**
 * Walk through the free space in the file, adding up the size of the
 * blocks on the way.
 *
 * \param *offset	Offset to the free space block to process.
 * \return		The amount of free space in the block and
 *			any blocks that are linked from it.
 */
static int32_t stronghelp_walk_free_space(int32_t offset)
{
	struct stronghelp_file_free_block *free;

	/* Exit if the pointer is -1. */

	if (offset < 0)
		return 0;

	/* Extract details of the block. */

	free = stronghelp_get_block_address(offset, sizeof(struct stronghelp_file_free_block));
	if (free == NULL)
		return 0;

	printf("Found free block: Magic Word 0x%x\n", free->free);
	printf("Size: %d bytes\n", free->free_size);
	printf("Next Offset: %d\n", free->next_offset);

	if (free->free != STRONGHELP_FREE_WORD) {
		msg_report(MSG_BAD_FREE_MAGIC, free->free);
		return 0;
	}

	return stronghelp_walk_free_space(free->next_offset) + free->free_size;
}

/**
 * Return an address for an offset in the current file.
 *
 * \param offset	The offset value to translate.
 * \param min_size	The minimum size of the block.
 * \return		Pointer to the block, or NULL on failure.
 */

static void *stronghelp_get_block_address(int32_t offset, size_t min_size)
{
	if (stronghelp_file_root == NULL) {
		msg_report(MSG_NO_FILE);
		return NULL;
	}

	if (offset < 0) {
		msg_report(MSG_BAD_OFFSET, offset);
		return NULL;
	}

	if (min_size < 0) {
		msg_report(MSG_BAD_SIZE, min_size);
		return NULL;
	}

	if (offset + min_size >= stronghelp_file_length) {
		msg_report(MSG_OFFSET_RANGE, offset, min_size, stronghelp_file_length);
		return NULL;
	}

	return stronghelp_file_root + offset;
}
