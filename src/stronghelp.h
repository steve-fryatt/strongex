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
 * \file stronghelp.h
 *
 * StrongHelp File Utilities, interface.
 */

#ifndef STRONGEX_STRONGHELP_H
#define STRONGEX_STRONGHELP_H

#include <stdlib.h>
#include <stdint.h>

/* Initialise a StrongHelp file and roughly validate its
 * contents.
 *
 * \param *file		Pointer to the file in memory.
 * \param length	The length of the file.
 */

void stronghelp_initialise_file(int8_t *file, size_t length);

#endif
