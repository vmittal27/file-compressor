#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <libgen.h> // Include for basename
#include <unistd.h> // for argument parsing
#include <limits.h>

#include "compress.h"
#include "decompress.h"
#include "string_table.h"
#define MAX_BITS_UB 20 // Maximum value for max_bits.
#define MAX_BITS_LB 9 // Minimum value for max_bits.
#define MAX_BITS_DEFAULT 12

int main(int argc, char *argv[])
{
    static char bin[64], bout[64];
    setvbuf(stdin, bin, _IOFBF, 64);
    setvbuf(stdout, bout, _IOFBF, 64);

    char *exec_name = basename(argv[0]); // Get the executable name

    if (strcmp(exec_name, "compress") == 0) {
        int max_bits = MAX_BITS_DEFAULT;
        
        int c;
        int arg;

        // Using getopt to parse command line options
        // m: indicates m takes an argument
        while ((c = getopt(argc, argv, "m:p")) != -1) {
            switch (c) {
                case 'm':
                    arg = atoi(optarg);
                    if (MAX_BITS_LB <= arg && arg <= MAX_BITS_UB)
                        max_bits = arg;
                    break;
                case '?':
                    fprintf(stderr, "compress: unknown option or missing argument");
                    exit(1); 
            }
        }

        compress(max_bits);
    } else if (strcmp(exec_name, "decompress") == 0) {
        if (argc > 1) {
            fprintf(stderr, "decompress: invalid option '%s'\n", argv[1]);
            exit(1);
        }
        decompress();
    } else {
        fprintf(stderr, "Usage: %s [-m MAXBITS] < input > output\n", argv[0]);
        fprintf(stderr, "       %s < input > output\n", argv[0]);
        exit(1);
    }

    exit(0);
}
