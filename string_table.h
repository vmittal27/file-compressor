#ifndef STRING_TABLE
#define STRING_TABLE
#define FNV_PRIME_64 0x00000100000001b3
#define FNV_OFFSET_BASIS_64 0xcbf29ce484222325

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

struct strtable_entry {
    int prefix;
    int character; 
    int code;
}; 

typedef struct strtable_entry strtable_entry; 

struct hashed_strtable_entry {
    strtable_entry data;
    struct hashed_strtable_entry *next; 
    
};

typedef struct hashed_strtable_entry hashed_strtable_entry; 

/*Implementation of the string table for compression, using FNV-1a hashing of a 
(prefix, character) pair to get the associated code.*/
struct compression_strtable {
    size_t size;
    size_t max_size; 

    size_t num_buckets;
    hashed_strtable_entry **buckets; 
};

typedef struct compression_strtable compression_strtable; 


/*Implementation of the string table for decompression, using an array of
(prefix, character) pairs indexed on the code for fast lookup given a code.*/
struct decompression_strtable {
    size_t size;
    size_t max_size; 

    strtable_entry *arr;
};

typedef struct decompression_strtable decompression_strtable;

/*Constructs a new compression string table given a max_size.
The returned string table is dynamically allocated and therefore must be freed.
The compression string table is implemented as a hash table.*/
compression_strtable *compression_strtable_new(size_t max_size);

/*Inserts a (prefix, character) pair into the table, assigning it the lowest available code.
This operation fails if the table is full.*/
void compression_strtable_insert(compression_strtable *table, int prefix, int character);

/*Retrieves the string table entry given a (prefix, character) pair.
The retrieved string table entry is a pointer. If the entry dosen't exist, returns NULL.*/
strtable_entry *compression_strtable_get(compression_strtable *table, int prefix, int character); 

/*Prunes the string table by removing any table entries that weren't used.*/
compression_strtable *compression_strtable_prune(compression_strtable *original);

/*Dumps a human readable version of the string table to the file specified.*/
void compression_strtable_dump(compression_strtable *table, char *filename);

/*Frees the memory allocated for a the string table.*/
void compression_strtable_free(compression_strtable *table);

/*Constructs a new string table to be used with decompression given a max_size*/
decompression_strtable *decompression_strtable_new(size_t max_size);

/*Inserts a (prefix, character) pair into the table, assigning it the lowest available code.
This operation fails if the table is full. */
void decompression_strtable_insert(decompression_strtable *table, int prefix, int character);


/*Given a code, retrieves the table entry associated with the code. 
Returns NULL if the code isn't in the table.*/
strtable_entry *decompression_strtable_get(decompression_strtable* table, int code);

/*Prunes the string table by removing any table entries that weren't used.*/
decompression_strtable *decompression_strtable_prune(decompression_strtable *original);

/*Dumps a human readable version of the string table to the file specified.*/
void decompression_strtable_dump(decompression_strtable* table, char* filename);

/*Frees the memory allocated for a the string table.*/
void decompression_strtable_free(decompression_strtable *table);

#endif
