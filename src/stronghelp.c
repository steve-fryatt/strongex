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

/* Local source headers. */

#include "stronghelp.h"

#include "msg.h"

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
	char		*filename;
			/* The filename follows this block,
			 * zero terminated and padded
			 * to a word boundary.
			 */
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
	struct stronghelp_file_root *root;
	int32_t free_space = 0;

	stronghelp_file_root = file;
	stronghelp_file_length = length;

	printf("Accepted file at 0x%x of %d bytes\n", file, length);

	root = (struct stronghelp_file_root *) stronghelp_file_root + 0;

	printf("Header: 0x%x\n", root->help);
	printf("Version: %d\n", root->version);
	printf("Size: %d\n", root->size);
	printf("Free Space offset: %d\n", root->free_offset);

	free_space = stronghelp_walk_free_space(root->free_offset);

	printf ("Total Free Space: %d bytes\n", free_space);
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

	printf("Found free block: header 0x%x\n", free->free);
	printf("Size: %d bytes\n", free->free_size);
	printf("Next Offset: %d\n", free->next_offset);

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

	if (offset + min_size >= stronghelp_file_length) {
		msg_report(MSG_OFFSET_RANGE, offset, min_size, stronghelp_file_length);
		return NULL;
	}

	return stronghelp_file_root + offset;
}
