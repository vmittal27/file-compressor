
#include "string_table.h"

/*
===============================================================================
UTILITY FUNCTIONS FOR STRING TABLE THAT ARE NOT EXPOSED IN THE HEADER FILE.
===============================================================================
*/

/*Uses the Fowler-Noll-Vo algorithm to hash the prefixes and characters*/
u_int64_t __hash_func(int prefix, int character) {
    u_int64_t hash = FNV_OFFSET_BASIS_64; 
    unsigned char *bytes; 

    // First, we hash the prefix.
    bytes = (unsigned char*)&prefix; 
    for (size_t i = 0; i < sizeof(int); i++) {
        hash ^= bytes[i]; 
        hash *= FNV_PRIME_64; 
    }
    // Next, we hash the character.
    bytes = (unsigned char*) &character;
    for (size_t i = 0; i < sizeof(int); i++) {
        hash ^= bytes[i]; 
        hash *= FNV_PRIME_64; 
    }
    
    return hash; 
}

/*Creates a dynamically-allocated deep copy of entry.*/
strtable_entry *__deepcopy_strtable_entry(strtable_entry *entry) {
    strtable_entry *copy = malloc(sizeof(strtable_entry));
    copy->character = entry->character;
    copy->prefix = entry->prefix;
    copy->code = entry->code;
    return copy; 
}

/*Converts a hash-table based string table to an array based string table. 
The returned table can therefore be traversed sequentially by code.*/
decompression_strtable *__compression_to_decompression_strtable(compression_strtable *table) {

    decompression_strtable *decompress_table = decompression_strtable_new(table->max_size); 


    for (int i = 0; i < table->num_buckets; i++) {

        hashed_strtable_entry *bucket = table->buckets[i];

        while (bucket) { // traverse each bucket

            decompress_table->arr[bucket->data.code] = bucket->data;

            decompress_table->size++;
            bucket = bucket->next; 
        }
    }

    return decompress_table;
}

/*
===============================================================================
COMPRESSION STRING TABLE
===============================================================================
*/

compression_strtable *compression_strtable_new(size_t max_size) {
    compression_strtable *table = malloc(sizeof(compression_strtable)); 

    // We set the hash table size to be 1.33x the max number of entries.
    size_t num_buckets = (4*max_size)/3; 

    // initialize fields
    table->num_buckets = num_buckets; 
    table->size = 0; 
    table->max_size = max_size;
    table->buckets = malloc(num_buckets*sizeof(hashed_strtable_entry*));

    for (int i = 0; i < num_buckets; i++) {
        table->buckets[i] = NULL; // initialize all buckets with NULL
    }

    return table;
}

void compression_strtable_insert(compression_strtable *table, int prefix, int character) {

    // In the case that the table is full, we can't insert.
    if (table->size >= table->max_size) {
        return; 
    }

    u_int64_t hash = __hash_func(prefix, character); 
    size_t index = hash % table->num_buckets; 

    hashed_strtable_entry *node = malloc(sizeof(hashed_strtable_entry)); 

    node->next = NULL;
    node->data = (strtable_entry) {
        .character = character,
        .prefix = prefix,
        .code = table->size++
    }; 

    hashed_strtable_entry *bucket = table->buckets[index]; 
    if (!bucket) // if bucket is empty
        table->buckets[index] = node;
    else { // puts node in the front of the list
        node->next = bucket; 
        table->buckets[index] = node; 
    }
}

strtable_entry *compression_strtable_get(compression_strtable *table, int prefix, int character) {

    // we first get the bucket the code lives in
    u_int64_t hash = __hash_func(prefix, character); 
    hashed_strtable_entry *bucket = table->buckets[hash % table->num_buckets]; 
    

    // then we sequentially check each node to see if the 
    // (prefix, character) pairs match to get the right code
    while (bucket != NULL) {
        if (bucket->data.prefix == prefix && bucket->data.character == character) {
            return &(bucket->data);
        } 
        bucket = bucket->next;
    }

    return NULL; // we return -1 if there is no match
    // this causes no concerns with representing the code -1 as 
    // empty because we never store (the code) -1 in the string table
}

compression_strtable *compression_strtable_prune(compression_strtable *original) {

    // we want to be able to get the prefix, character pair
    // based on the code, so we create a decode array to help with that
    decompression_strtable *decompress_table = __compression_to_decompression_strtable(original); 

    // we first find the codes that we will keep
    // by traversing the entire hash table
    int *codes_to_keep = calloc(original->size, sizeof(int));

    // although we don't formally need this
    // keeping track of the pruned size adds minimal overhead
    // and is helpful for debugging
    int pruned_size = 0; 

    for (int i = 0; i < original->num_buckets; i++) {

        hashed_strtable_entry *bucket = original->buckets[i];

        while (bucket) {

            int prefix = bucket->data.prefix;
            if (prefix == -1) {
                if (codes_to_keep[bucket->data.character] == 0) {
                    codes_to_keep[bucket->data.character] = 1;
                    pruned_size++;
                }
            }
            else if (codes_to_keep[prefix] == 0) {
                codes_to_keep[prefix] = 1; 
                pruned_size++; 
            }

            bucket = bucket->next; 
        }
    }

    // now we need to reconstruct the hash table

    // we maintain a mapping of the old codes to the new codes
    // since some codes may change
    // the index of the array represents the old code
    // and the value of the array element represents the new code
    int *old_to_new_codes = calloc(original->size, sizeof(int));

    compression_strtable *pruned_table = compression_strtable_new(original->max_size); 

    // now we populate the pruned hash table
    for (int i = 0; i < original->size; i++) {
        if (codes_to_keep[i]) { // we should keep this code
            strtable_entry data = decompress_table->arr[i]; // we get the prefix, character pair
            old_to_new_codes[i] = pruned_table->size;

            if (data.prefix == -1) // it is one of the 256 base characters
                compression_strtable_insert(pruned_table, data.prefix, data.character);
            else // we need to account for the fact that the prefix code may have changed
                compression_strtable_insert(pruned_table, old_to_new_codes[data.prefix], data.character);
        }
    }

    // we free the decode arr we made
    decompression_strtable_free(decompress_table); 
    free(old_to_new_codes);
    free(codes_to_keep);

    return pruned_table;

}

void compression_strtable_dump(compression_strtable *table, char *filename) {

    // turns hash_table into a temporary array because codes are sorted
    // and it is easier to represent the strings

    decompression_strtable *decompress_table = __compression_to_decompression_strtable(table); 
    decompression_strtable_dump(decompress_table, filename); 
    decompression_strtable_free(decompress_table);
}

void compression_strtable_free(compression_strtable *table) {
    for (int i = 0; i < table->num_buckets; i++) {
        hashed_strtable_entry* bucket = table->buckets[i];
        while (bucket) {
            hashed_strtable_entry *next = bucket->next;
            free(bucket);
            bucket = next; 
        }
    }
    free(table->buckets); 
    free(table); 
}

/*
===============================================================================
DECOMPRESSION STRING TABLE
===============================================================================
*/

decompression_strtable *decompression_strtable_new(size_t max_size) {
    decompression_strtable* table = malloc(sizeof(decompression_strtable)); 
    table->size = 0; 
    table->max_size = max_size; 
    table->arr = calloc(max_size, sizeof(strtable_entry)); 
    return table; 
}

void decompression_strtable_insert(decompression_strtable *table, int prefix, int character) {
    // In the case that the table is full, we can't insert.
    if (table->size >= table->max_size) {
        return; 
    }
    table->arr[table->size] = (strtable_entry) {.prefix = prefix, .character = character, .code = table->size};   
    table->size++;  
}

strtable_entry *decompression_strtable_get(decompression_strtable* table, int code) {
    if (code < 0 || code >= table->size)
        return NULL;
    return &(table->arr[code]); 
}

decompression_strtable *decompression_strtable_prune(decompression_strtable *original) {
    // we first find the codes that we will keep
    // by traversing the entire array
    int *codes_to_keep = calloc(original->size, sizeof(int));

    // although we don't formally need this
    // keeping track of the pruned size adds minimal overhead
    // and is helpful for debugging
    int pruned_size = 0;

    for (int i = 0; i < original->size; i++) {
        int prefix = original->arr[i].prefix;

        if (prefix == -1 && codes_to_keep[original->arr[i].character] == 0) {
            codes_to_keep[original->arr[i].character] = 1; 
            pruned_size++; 
        }
        else if (codes_to_keep[prefix] == 0) {
            codes_to_keep[prefix] = 1;
            pruned_size++;
        }
    }

    // we'll now reconstruct the array

    decompression_strtable *pruned_table = decompression_strtable_new(original->max_size); 

    // we maintain a mapping of the old codes to the new codes
    // since some codes may change
    int *old_to_new_codes = calloc(original->size, sizeof(int));

    // now we populate the pruned array
    for (int i = 0; i < original->size; i++) {
        if (codes_to_keep[i]) { // we should keep this code
            strtable_entry data = original->arr[i]; // we get the prefix, character pair
            old_to_new_codes[i] = pruned_table->size;

            // we may have to adjust the prefix since codes can change
            int prefix = data.prefix == -1 ? data.prefix : old_to_new_codes[data.prefix]; 

            decompression_strtable_insert(pruned_table, prefix, data.character); 
        }
    }
    free(old_to_new_codes);
    free(codes_to_keep);
    return pruned_table;

}

void decompression_strtable_free(decompression_strtable* table) {
    free(table->arr); 
    free(table);
}


void decompression_strtable_dump(decompression_strtable* arr, char *filename) {
    FILE *file = fopen(filename, "w"); 
    fprintf(file, "String Table Dump\n");
    fprintf(file, "%-8s\t%-8s\t%-12s\t%-8s\n", "Code", "Prefix", "Character", "String");
    for (int i = 0; i < arr->size; i++) {
        // we first convert all the prefixes to their base characters
        int prefix = i; 
        int length = 0; 
    
        int buffer[arr->size]; 
        memset(buffer, 0, arr->size);

        if (arr->arr[i].prefix == i)
            buffer[length++] = arr->arr[i].character; 
        else {
            while (prefix != -1) {
                buffer[length++] = arr->arr[prefix].character;
                prefix = arr->arr[prefix].prefix;
        }

        }
        // since we appended prefixes, we have to then reverse the array

        for (int j = 0; j < length/2; j++) {
            int temp = buffer[j];
            buffer[j] = buffer[length - 1 - j];
            buffer[length - 1 - j] = temp; 
        }

        // now we turn an array of ints into a string
        // with their integer representations
        char s[12*arr->size]; 
        int s_len = 0; 
        for (int k = 0; k < length; k++) {
            s_len += sprintf(s + s_len, "%d ", buffer[k]); 
        }

        fprintf(
            file, 
            "%-8d\t%-8d\t%-12d\t%-8s\n", 
            i, 
            arr->arr[i].prefix,
            arr->arr[i].character, 
            s
        );

    }    
    fclose(file);
}
