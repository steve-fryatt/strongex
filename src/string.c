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
 * \file string.c
 *
 * String Utilities, implementation.
 */

#include <ctype.h>
#include <stddef.h>

/* Local source headers. */

#include "string.h"


/* Perform a strcmp() case-insensitively on two strings, returning
 * a value less than, equal to or greater than zero depending on
 * their relative values.
 *
 * \param *s1		The first string to be compared.
 * \param *s2		The second string to be compared.
 * \return		The result of the comparison.
 */

int string_nocase_strcmp(char *s1, char *s2)
{
	while (*s1 != '\0' && *s2 != '\0' && (toupper(*s1) - toupper(*s2)) == 0) {
		s1++;
		s2++;
	}

	return (toupper(*s1) - toupper(*s2));
}

/* Perform a strncpy(), sanity-checking the supplied pointer details and
 * ensuring that the copy is zero-terminated even if the source string
 * is longer than the supplied buffer.
 *
 * \param *dest		A buffer to hold the copied string.
 * \param *src		The string to be copied.
 * \param len		The maximum number of characters to copy.
 * \return		A pointer to the copy of the string, or NULL
 *			if the supplied pointers were invalid.
 */

char *string_copy(char *dest, char *src, size_t len)
{
	char *ret;

	if (dest == NULL || src == NULL || len == 0)
		return NULL;

	ret = strncpy(dest, src, len);
	dest[len - 1] = '\0';

	return ret;
}

/**
 * Append one string on to the end of another.
 *
 * \param *buffer	The buffer into which to copy the string.
 * \param *target	The string to be copied.
 * \param length	The length of the buffer, including any
 *			characters already there.
 */

void string_append(char *buffer, char *source, size_t length)
{
	if (buffer == NULL || source == NULL || length <= 0)
		return;

	while (*buffer != '\0' && length-- > 0)
		buffer++;

	while (*source != '\0' && length-- > 0)
		*(buffer++) = *(source++);

	if (length == 0)
		buffer--;

	*buffer = '\0';
}
