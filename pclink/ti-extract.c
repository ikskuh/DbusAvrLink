#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <ctype.h>

#include "ti83f.h"
#include "debug.h"

int main(int argc, char ** argv)
{
	int index = 1;
	
	int c;
	while ((c = getopt (argc, argv, "hn:o:")) != -1)
	{
		switch (c)
		{
		case 'h': 
			printf("%s [-n index] [-o fileName] [inputFile]\n", argv[0]);
			exit(EXIT_SUCCESS);
		case 'n':
			index = atoi(optarg);
			break;
		case 'o':
			if(freopen(optarg, "wb", stdout) == NULL) {
				error_message("Could not write to %s\n", optarg);
				exit(EXIT_FAILURE);
			}
			break;
		case '?':
			if (optopt == 'c')
				fprintf (stderr, "Option -%c requires an argument.\n", optopt);
			else if (isprint (optopt))
				fprintf (stderr, "Unknown option `-%c'.\n", optopt);
			else
				fprintf (stderr,
					"Unknown option character `\\x%x'.\n",
					optopt);
			exit(EXIT_FAILURE);
		default:
			abort ();
		}
	}
	
	if(optind == argc) {
		// Read from stdin
	}
	else if(optind < (argc - 1)) {
		fprintf(stderr, "Only one file is allowed.\n");
		exit(EXIT_FAILURE);
	}
	else {
		freopen(argv[optind], "rb", stdin);
	}
	
	struct ti83f_file * file = ti83f_load(stdin);
	fclose(stdin);
	
	if(file == NULL) {
		error_message("Input is not a TI83F file.\n");
		exit(EXIT_FAILURE);
	}
	
	if(index <= 0 || index > file->length) {
		error_message("The file does not contain %d entries.\n", index);
		exit(EXIT_FAILURE);
	}

	struct ti83f_entry * entry = &file->entries[index - 1];
	
	if(entry->data == NULL) {
		error_message("The selected entry does not contain any data.\n");
		exit(EXIT_FAILURE);
	}
	
	fwrite(entry->data, 1, entry->size, stdout);
	
	fclose(stdout);

	return 0;
}