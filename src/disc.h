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
 * \file disc.h
 *
 * Disc File Utilities, Interface.
 */

#ifndef STRONGEX_DISC_H
#define STRONGEX_DISC_H

#include <stdbool.h>

/* Initialise a folder on disc, and roughly validate its
 * contents.
 *
 * \param *file		Pointer to the folder path.
 * \return		True if successful, false on failure.
 */

bool disc_initialise_folder(char *path);

#endif

