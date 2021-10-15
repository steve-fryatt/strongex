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
#include <stdio.h>

/* Local source headers. */

#include "args.h"
#include "msg.h"

/* OSLib source headers. */

#ifdef RISCOS
#include "oslib/osfile.h"
#endif

#define MAX_INPUT_LINE_LENGTH 1024
#define MAX_LOCATION_TEXT 256

static bool strongex_process_file(char *source_file, char *output_folder, bool verbose_output);

int main(int argc, char *argv[])
{
	bool			param_error = false;
	bool			output_help = false;
	bool			verbose_output = false;
	char			*source_file = NULL;
	char			*output_folder = NULL;
	struct args_option	*options;
	struct args_data	*option_data;

	/* Default processing options. */


	/* Initialise the variable and procedure handlers. */


	/* Decode the command line options. */

	options = args_process_line(argc, argv,
			"source/A,out/A,verbose/S,help/S");
	if (options == NULL)
		param_error = true;

	while (options != NULL) {
		if (strcmp(options->name, "help") == 0) {
			if (options->data != NULL && options->data->value.boolean == true)
				output_help = true;
		} else if (strcmp(options->name, "verbose") == 0) {
			if (options->data != NULL && options->data->value.boolean == true)
				verbose_output = true;
		} else if (strcmp(options->name, "source") == 0) {
			if (options->data != NULL) {
				option_data = options->data;

				while (option_data != NULL) {
					if (option_data->value.string != NULL)
						source_file = option_data->value.string;
					option_data = option_data->next;
				}
			} else {
				param_error = true;
			}
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
		printf("Copyright Stephen Fryatt, 2014-%s\n", BUILD_DATE + 7);
	}

	if (param_error || output_help) {
		printf("StrongHelp Manual Extractor -- Usage:\n");
		printf("strongex <infile> [<infile> ...] -out <outfile> [<options>]\n\n");

		printf(" -help                  Produce this help information.\n");
		printf(" -out <file>            Write tokenized basic to file <out>.\n");
		printf(" -verbose               Generate verbose process information.\n");

		return (output_help) ? EXIT_SUCCESS : EXIT_FAILURE;
	}

	/* Run the tokenisation. */

	if (!strongex_process_file(source_file, output_folder, verbose_output) || msg_errors())
		return EXIT_FAILURE;

	return EXIT_SUCCESS;
}


/**
 * Process a StrongHelp file, reading the data from the source and
 * writing the files that it contains to the specified output folder.
 *
 * \param *source_file		Pointer to the name of the file to read from.
 * \param *output_folder	Pointer to the name of the folder to write to.
 * \param *verbose_output	True if verbose output should be generated.
 * \return			True on success; false on failure.
 */

static bool strongex_process_file(char *source_file, char *output_folder, bool verbose_output)
{
	FILE		*in;
	bool		success = true;
	int		length = 0;
	char		*buffer = NULL;

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

	if (buffer != NULL) {
		if (fread(buffer, sizeof(char), length, in) != length) {
			free(buffer);
			buffer = NULL;
		}
	}

	fclose(in);

	if (buffer == NULL) {
		printf("Failed to read StrongHelp file into memory.\n");
		return false;
	}

	printf("Read %d bytes of data.\n", length);

//#if RISCOS
//	osfile_set_type(output_file, osfile_TYPE_BASIC);
//#endif

	return success;
}

#if 0
/**
 * Tokenise the contents of a file, sending the results to the output.
 *
 * \param *in		The handle of the file to be tokenised.
 * \param *out		The handle of the file to write the output to.
 * \param *line_number	Pointer to a variable holding the current line number.
 * \param *options	Pointer to the tokenisation options.
 * \return		True on success; false if an error occurred.
 */

static bool tokenize_parse_file(FILE *in, FILE *out, int *line_number, struct parse_options *options)
{
	char		line[MAX_INPUT_LINE_LENGTH], *tokenised, *file;
	bool		assembler = false;
	unsigned	input_line = 0;

	if (in == NULL || out == NULL || line_number == NULL || options == NULL)
		return false;

	file = library_get_filename();
	if (file == NULL)
		file = "unknown file";

	if (options->verbose_output)
		printf("Processing source file '%s'\n", file);

	while (tokenize_fgets(line, MAX_INPUT_LINE_LENGTH - 1, in) != NULL) {
		msg_set_location(++input_line, file);

		tokenised = parse_process_line(line, options, &assembler, line_number);
		if (tokenised != NULL) {
			/* The line tokeniser requests a line be deleted (ie. not
			 * written to the output) by setting the leading \r to be
			 * \0 instead (setting the line pointer to NULL signifies
			 * an error).
			 */

			if (*tokenised != '\0')
				fwrite(tokenised, sizeof(char), *((unsigned char *) tokenised + 3), out);
		} else {
			return false;
		}
	}

	return true;
}
#endif
