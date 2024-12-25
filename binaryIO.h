/*
Custom library for working with binary I/O in C.
Supports reading and writing from a file stream at the granularity of bits.
*/
#ifndef BINARY_IO
#define BINARY_IO
#include <stdio.h>
#include <stdint.h>
#define BUFFER_MAX 8
#define CHAR_BIT 8

struct binaryio_buffer {
    unsigned char buffer; 
    int size;
}; 

typedef struct binaryio_buffer binaryio_buffer; 

/*
Attempts to add a singular bit to the binary buffer.
Bits are added to the right of a binary buffer.
If the buffer is full, this operation fails and returns -1.
If it is not full, the bit is added and returns 0.
*/
int binaryio_buffer_push(binaryio_buffer *b_buffer, int bit);

/*
Attempts to pop the leftmost bit from the binary buffer.
This operation fails and returns -1 when the buffer is empty.
Otherwise, it returns the popped bit. 
*/
int binaryio_buffer_pop(binaryio_buffer *b_buffer);

// Clears a binary buffer.
void binaryio_buffer_clear(binaryio_buffer *b_buffer);

/*
Writes the binary buffer to the specified stream.
If the binary buffer is not full, the buffer will be padded with 0's in the back.
This operation clears the binary buffer after writing to the stream.
*/
void binaryio_buffer_write(FILE* stream, binaryio_buffer *b_buffer);

/*
Writes an integer to a specified stream in binary mode
using the amount of bits specified by num_bits and the
specified bin_buffer as the buffer. If flush is set to true, 
whatever remains in the buffer is sent to the file stream 
even if the buffer is not full (in which case it is padded with 0's)*/
void binary_write(FILE* stream, binaryio_buffer *b_buffer, int data, int num_bits, int flush);

/* Reads num_bits from the specified stream, storing an integer representation of the bits in data.
Supports buffered operations by first trying to read from the binary buffer, and then reading the rest
from the stream. Since stream is read 1 byte at a time, any bits not consumed will be stored in reverse order
within the bin_buffer. Returns 1 if it sucessfully read the specified number of bits, -1 otherwise. 
*/
int binary_read(FILE* stream, binaryio_buffer *b_buffer, int *data, int num_bits); 

#endif