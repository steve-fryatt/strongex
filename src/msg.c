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
 * \file msg.c
 *
 * Status Message, implementation.
 */

#include <ctype.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <stdarg.h>

/* Local source headers. */

#include "msg.h"

#define MSG_MAX_LOCATION_TEXT 256
#define MSG_MAX_MESSAGE 256

enum msg_level {
	MSG_INFO,
	MSG_WARNING,
	MSG_ERROR
};

struct msg_data {
	enum msg_level	level;
	char		*text;
	bool		show_location;
};


/**
 * Error message definitions.
 *
 * NB: The order of these messages *must* match the order of the corresponding
 * entries in enum msg_type in msgs.h
 */

static struct msg_data msg_messages[] = {
	{MSG_ERROR,	"Unknown error",				true	}
};

static char	msg_location[MSG_MAX_LOCATION_TEXT];

/**
 * Set to true if an error is reported.
 */

static bool	msg_error_reported = false;

/**
 * Set the location for future messages, in the form of a file and line number
 * relating to the source files.
 *
 * \param line		The number of the current line.
 * \param *file		Pointer to the name of the current file.
 */

void msg_set_location(unsigned line, char *file)
{
	snprintf(msg_location, MSG_MAX_LOCATION_TEXT, "at line %u of '%s'", line, (file != NULL) ? file : "");
	msg_location[MSG_MAX_LOCATION_TEXT - 1] = '\0';
}


/**
 * Generate a message to the user, based on a range of standard message tokens
 *
 * \param type		The message to be displayed.
 * \param ...		Additional printf parameters as required by the token.
 */

void msg_report(enum msg_type type, ...)
{
	char		message[MSG_MAX_MESSAGE], *level;
	va_list		ap;

	if (type < 0 || type >= MSG_MAX_MESSAGES)
		return;

	va_start(ap, type);
	vsnprintf(message, MSG_MAX_MESSAGE, msg_messages[type].text, ap);
	va_end(ap);

	message[MSG_MAX_MESSAGE - 1] = '\0';

	switch (msg_messages[type].level) {
	case MSG_INFO:
		level = "Info";
		break;
	case MSG_WARNING:
		level = "Warning";
		break;
	case MSG_ERROR:
		level = "Error";
		msg_error_reported = true;
		break;
	default:
		level = "Message:";
		break;
	}

	if (msg_messages[type].show_location)
		fprintf(stderr, "%s: %s %s\n", level, message, msg_location);
	else
		fprintf(stderr, "%s: %s\n", level, message);
}


/**
 * Indicate whether an error has been reported at any point.
 *
 * \return		True if an error has been reported; else false.
 */

bool msg_errors(void)
{
	return msg_error_reported;
}

