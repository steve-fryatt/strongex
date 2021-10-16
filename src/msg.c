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

#define MSG_MAX_MESSAGE 256

enum msg_level {
	MSG_INFO,
	MSG_WARNING,
	MSG_ERROR
};

struct msg_data {
	enum msg_level	level;
	char		*text;
};


/**
 * Error message definitions.
 *
 * NB: The order of these messages *must* match the order of the corresponding
 * entries in enum msg_type in msgs.h
 */

static struct msg_data msg_messages[] = {
	{MSG_ERROR,	"Unknown error"},
	{MSG_ERROR,	"Out of memory"},
	{MSG_ERROR,	"Failed to read file '%s' into memory"},
	{MSG_ERROR,	"No file currently loaded"},
	{MSG_ERROR,	"Attempt to use invalid offset of %d"},
	{MSG_ERROR,	"Attempt to use invalid size of %d"},
	{MSG_ERROR,	"Offset %d and block size %d bytes is outside file size of %d bytes"},
	{MSG_ERROR,	"Unable to locate directory entry"},
	{MSG_ERROR,	"Unexpected file magic word 0x%x"},
	{MSG_ERROR,	"Unexpected free magic word 0x%x"},
	{MSG_ERROR,	"Unexpected object magic word 0x%x"},
	{MSG_ERROR,	"Unable to find root directory entry"},
	{MSG_ERROR,	"Attempt to create multiple root directories"},
	{MSG_ERROR,	"No parent directory specified"}
};

/**
 * Set to true if an error is reported.
 */

static bool	msg_error_reported = false;


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
