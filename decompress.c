#include "decompress.h"
#include "string_table.h"
#include "stack.h"
#include "stdio.h"
#include "binaryIO.h"
#define ASCII_CHAR_MAX 256
#define MAX_BITS_DEFAULT 12

void decompress() {

    int max_bits = MAX_BITS_DEFAULT;

    binaryio_buffer b_buf = {.buffer = 0, .size = 0}; 
    int cur_bits = 9;
    int cur_max = 1 << 9; 

    // We first get the max bits and whether or not we prune.
    binary_read(stdin, &b_buf, &max_bits, 5);

    // Max size for the string table.
    int max_size = 1 << max_bits; 

    decompression_strtable *table = decompression_strtable_new(max_size); 

    // Initializes 8-bit characters to the string table.
    for (int i = 0; i < ASCII_CHAR_MAX; i++) {
        decompression_strtable_insert(table, -1, i);
    }

    stack *char_stack = stack_new(); 

    int old_code = -1; // -1 represents EMPTY
    int next_code; 
    int code; 

    /*Code reading occurs after pruning,
    to ensure synchronization between encode and decode string tables.
    This results in a while True loop with a break.*/
    while (1) { 

        if (table->size >= table->max_size) {
            decompression_strtable *pruned_table = decompression_strtable_prune(table);
            decompression_strtable *original = table;
            table = pruned_table;
            decompression_strtable_free(original);
    
            // We may be able to represent codes
            // with less bits now, so we check for that.
            int cur_size = table->size; 
            int new_bits = 0;
            while (cur_size) {
                cur_size >>= 1; 
                new_bits++; 
            }
            cur_bits = new_bits;
            cur_max = 1 << cur_bits; 
            if (table->size + 1 == cur_max) {
                cur_bits++;
                cur_max *= 2; 
            }
    
        }
        int status = binary_read(stdin, &b_buf, &next_code, cur_bits);

        if (status != 1)
            break;

        code = next_code; 

        /*In the case of an unknown code, we find the final character
        of the code and push it onto the stack.*/
        if (code >= table->size) {

            int temp = old_code;
            while (table->arr[temp].prefix != -1) {
                temp = table->arr[temp].prefix;
            }
            stack_push(char_stack, table->arr[temp].character);

            code = old_code;
        }

        while (table->arr[code].prefix != -1) {
            stack_push(char_stack, table->arr[code].character); 
            code = table->arr[code].prefix;
        }

        int final_char = table->arr[code].character; 

        printf("%c", final_char); 

        while (char_stack->size > 0) {
            int intermediate_char = stack_pop(char_stack); 
            printf("%c", intermediate_char); 
        }

        // We add to the string table
        // BUT, only if we have room for more codes.
        if (old_code != -1) {
            decompression_strtable_insert(table, old_code, final_char);
        }

        old_code = next_code; 

        // In the event that we need to represent codes with more bits,
        // we do so here.
        if ((table->size + 1) >= cur_max && cur_bits < max_bits) {
            cur_bits++;
            cur_max *= 2; 
        }

    }

    if (getenv("DBG") != NULL && strcmp(getenv("DBG"), "1") == 0)
        decompression_strtable_dump(table, "./DBG.decompress"); 
    
    decompression_strtable_free(table);
    stack_free(char_stack);

}
