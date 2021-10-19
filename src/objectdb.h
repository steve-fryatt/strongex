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
 * \file objectdb.h
 *
 * Object Database Interface.
 */

#ifndef STRONGEX_OBJECTDB_H
#define STRONGEX_OBJECTDB_H

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
 * \return		Pointer to the new file instance, or NULL.
 */

struct objectdb_object *objectdb_add_stronghelp_file(struct objectdb_object *parent, char *name);

/**
 * Write a report of the objects held in the database.
 */

void objectdb_create_report(void);

#endif

