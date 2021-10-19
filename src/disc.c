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

/* Initialise a folder on disc, and roughly validate its
 * contents.
 *
 * \param *file		Pointer to the folder path.
 */

void disc_initialise_folder(char *path)
{
	struct files_object_info *objects;

	objects = files_read_directory_contents(path);

	while (objects != NULL) {
		printf("Found an object: %s, size=%d, filetype=0x%x, realname=%s\n", objects->name, objects->size, objects->filetype, objects->real_name);

		objects = objects->next;
	}
}