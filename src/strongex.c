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

/* Strong Extract
 *
 * Extract the files from within StrongHelp manuals.
 *
 * Syntax: strongex [<options>]
 *
 * Options -v  - Produce verbose output
 */

#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <stdio.h>

/* Local source headers. */

#include "args.h"
#include "disc.h"
#include "msg.h"
#include "objectdb.h"
#include "stronghelp.h"

/* OSLib source headers. */

#ifdef RISCOS
#include "oslib/osfile.h"
#endif

#define MAX_INPUT_LINE_LENGTH 1024
#define MAX_LOCATION_TEXT 256

static bool strongex_process_file(char *source_file, char *output_folder, bool output_all, bool verbose_output);

int main(int argc, char *argv[])
{
	bool			param_error = false;
	bool			output_help = false;
	bool			verbose_output = false;
	bool			output_all = false;
	char			*source_file = NULL;
	char			*output_folder = NULL;
	struct args_option	*options;
	struct args_data	*option_data;

	/* Default processing options. */


	/* Initialise the variable and procedure handlers. */


	/* Decode the command line options. */

	options = args_process_line(argc, argv,
			"all/S,source/A,out/A,verbose/S,help/S");
	if (options == NULL)
		param_error = true;

	while (options != NULL) {
		if (strcmp(options->name, "all") == 0) {
			if (options->data != NULL && options->data->value.boolean == true)
				output_all = true;
		} else if (strcmp(options->name, "help") == 0) {
			if (options->data != NULL && options->data->value.boolean == true)
				output_help = true;
		} else if (strcmp(options->name, "verbose") == 0) {
			if (options->data != NULL && options->data->value.boolean == true)
				verbose_output = true;
		} else if (strcmp(options->name, "source") == 0) {
			if (options->data != NULL && options->data->value.string != NULL)
				source_file = options->data->value.string;
			else
				param_error = true;
		} else if (strcmp(options->name, "out") == 0) {
			if (options->data != NULL && options->data->value.string != NULL)
				output_folder = options->data->value.string;
			else
				param_error = true;
		}

		options = options->next;
	}

	if (param_error || output_help || verbose_output) {
		printf("Strong Extract %s - %s\n", BUILD_VERSION, BUILD_DATE);
		printf("Copyright Stephen Fryatt, %s\n", BUILD_DATE + 7);
	}

	if (param_error || output_help) {
		printf("StrongHelp Manual Extractor -- Usage:\n");
		printf("strongex <infile> -out <outfolder> [<options>]\n\n");

		printf(" -all                   Include unchanged files in the report.\n");
		printf(" -help                  Produce this help information.\n");
		printf(" -out <folder>          Write manual contents to <folder>.\n");
		printf(" -verbose               Generate verbose process information.\n");

		return (output_help) ? EXIT_SUCCESS : EXIT_FAILURE;
	}

	/* Run the tokenisation. */

	if (!strongex_process_file(source_file, output_folder, output_all, verbose_output) || msg_errors())
		return EXIT_FAILURE;

	return EXIT_SUCCESS;
}


/**
 * Process a StrongHelp file, reading the data from the source and
 * writing the files that it contains to the specified output folder.
 *
 * \param *source_file		Pointer to the name of the file to read from.
 * \param *output_folder	Pointer to the name of the folder to write to.
 * \param output_all		Should the report show all files, or only changed ones.
 * \param *verbose_output	True if verbose output should be generated.
 * \return			True on success; false on failure.
 */

static bool strongex_process_file(char *source_file, char *output_folder, bool output_all, bool verbose_output)
{
	FILE		*in;
	bool		success = true;
	size_t		length = 0;
	int8_t		*buffer = NULL;

	if (source_file == NULL || output_folder == NULL)
		return false;

	if (verbose_output)
		printf("Extracting StrongHelp file '%s' to '%s'\n", source_file, output_folder);

	in = fopen(source_file, "r");
	if (in == NULL)
		return false;

	/* Get the size of the file. */

	fseek(in, 0, SEEK_END);
	length = ftell(in);
	fseek(in, 0, SEEK_SET);

	buffer = malloc(length);

	if ((buffer != NULL) && (fread(buffer, sizeof(char), length, in) != length)) {
		free(buffer);
		buffer = NULL;
	}

	fclose(in);

	if (buffer == NULL) {
		msg_report(MSG_LOAD_FAILED, source_file);
		return false;
	}

	if (verbose_output)
		printf("Read %d bytes of data to location 0x%x.\n", length, buffer);

	stronghelp_initialise_file(buffer, length);

	disc_initialise_folder(output_folder);

	objectdb_check_status();

	objectdb_output_report(output_all);

	free(buffer);

	return success;
}
