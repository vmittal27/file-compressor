#include "compress.h"
#include "binaryIO.h"
#include "string_table.h"
#include <limits.h>
#define ASCII_CHAR_MAX 256

void compress(int max_bits) {

    size_t max_size = 1 << max_bits; // Max size of string table.

    /*Pruning only occurs when MAXBITS is greater than 10 to minimize compression time
    on small string tables.*/
    int prune = (max_bits > 10) ? 1 : 0;

    binaryio_buffer b_buf = {.buffer = 0, .size = 0}; 
    int cur_bits = 9; // How many bits we need to represent each code.
    int cur_max = 1 << cur_bits;

    // We represent max bits with 5 bits
    binary_write(stdout, &b_buf, max_bits, 5, 0); 

    compression_strtable *str_table = compression_strtable_new(max_size); 

    // We first initialize the ASCII characters into the table.
    for (int i = 0; i < ASCII_CHAR_MAX; i++) {
        compression_strtable_insert(str_table, -1, i);
    }

    // We'll represent code = -1 as the empty string.
    // If we use 0, we get problems with binary files.
    int code = -1;
    int character;

    while ((character = getchar()) != EOF) {
        strtable_entry *match = compression_strtable_get(str_table, code, character); 

        // Checks if given (prefix, character) is in hash table.
        if (match != NULL) {
            code = match->code;
        }

        else {
            // If we need to represent codes with more bits.
            if (str_table->size >= cur_max && cur_bits < max_bits) {
                cur_bits++;
                cur_max *= 2; 
            }

            binary_write(stdout, &b_buf, code, cur_bits, 0); 

            // If the table is full, we prune.
            if (prune && str_table->size >= str_table->max_size) {
                compression_strtable *pruned_table = compression_strtable_prune(str_table);
                compression_strtable *original = str_table;
                compression_strtable_free(original);
                str_table = pruned_table;

                // Now we figure out how many bits to represent codes with.
                int cur_size = str_table->size; 
                int new_bits = 0;
                while (cur_size) {
                    cur_size >>= 1; 
                    new_bits++; 
                }
                cur_bits = new_bits;
                cur_max = 1 << cur_bits; 

            }

            compression_strtable_insert(str_table, code, character);
    
            strtable_entry* entry = compression_strtable_get(str_table, -1, character);
            code = (entry != NULL) ? entry->code : -1;
        }


    }
    // If we need to print out another code we do, flushing at the end.
    if (code != -1) 
        binary_write(stdout, &b_buf, code, cur_bits, 1); 
    // Otherwise we flush the buffer.
    else
        binaryio_buffer_write(stdout, &b_buf);
    
    // For debugging.
    if (getenv("DBG") != NULL && strcmp(getenv("DBG"), "1") == 0)
        compression_strtable_dump(str_table, "./DBG.compress");

    compression_strtable_free(str_table);


}