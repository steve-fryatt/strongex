/* Copyright 2014, Stephen Fryatt (info@stevefryatt.org.uk)
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
 * \file msg.h
 *
 * Status Message Interface.
 */

#ifndef STRONGEX_MSG_H
#define STRONGEX_MSG_H

#include <stdbool.h>


/**
 * Error message codes.
 *
 * NB: The order of these values *must* match the order of the error message
 * definitions in msg_definitions[] in msgs.c.
 */

enum msg_type {
	MSG_UNKNOWN_ERROR = 0,
	MSG_MAX_MESSAGES
};


/**
 * Set the location for future messages, in the form of a file and line number
 * relating to the source files.
 *
 * \param line		The number of the current line.
 * \param *file		Pointer to the name of the current file.
 */

void msg_set_location(unsigned line, char *file);


/**
 * Generate a message to the user, based on a range of standard message tokens
 *
 * \param type		The message to be displayed.
 * \param ...		Additional printf parameters as required by the token.
 */

void msg_report(enum msg_type type, ...);


/**
 * Indicate whether an error has been reported at any point.
 *
 * \return		True if an error has been reported; else false.
 */

bool msg_errors(void);

#endif

