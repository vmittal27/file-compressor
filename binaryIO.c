#include "binaryIO.h"

int binaryio_buffer_push(binaryio_buffer *b_buffer, int bit) {

    if (b_buffer->size == BUFFER_MAX)
        return -1;

    b_buffer->buffer = (b_buffer->buffer << 1) | (bit & 1);
    b_buffer->size++; 

    return 0;
}

int binaryio_buffer_pop(binaryio_buffer *b_buffer) {

    if (b_buffer->size == 0)
        return -1;
    // we get the leftmost bit
    int bit = (b_buffer->buffer >> (b_buffer->size - 1)) & 1; 

    // now we set the leftmost bit to 0
    int mask = 1 << (b_buffer->size - 1);
    b_buffer->buffer &= ~mask; 
    b_buffer->size--; 

    return bit; 
}

void binaryio_buffer_clear(binaryio_buffer *b_buffer) {
    b_buffer->size = 0;
    b_buffer->buffer = 0; 
}

void binaryio_buffer_write(FILE* stream, binaryio_buffer *b_buffer) {
    // we pad with 0s in the back if necessary
    b_buffer->buffer = b_buffer->buffer << (BUFFER_MAX - b_buffer->size); 

    fwrite(&(b_buffer->buffer), 1, 1, stream); 

    // we clear the buffer
    binaryio_buffer_clear(b_buffer); 

}

void binary_write(FILE* stream, binaryio_buffer *b_buffer, int data, int num_bits, int flush) {
    for (int i = num_bits - 1; i >= 0; i--) {

        int bit = (data >> i) & 1; 

        if (binaryio_buffer_push(b_buffer, bit) == -1) {
    
            binaryio_buffer_write(stream, b_buffer);
            binaryio_buffer_push(b_buffer, bit); 
        }
    }

    // in case we flush the buffer
    if (b_buffer->size > 0 && flush) {
        binaryio_buffer_write(stream, b_buffer);
    }

}

int binary_read(FILE* stream, binaryio_buffer *b_buffer, int *data, int num_bits) {
    int result = 0; 
    int i = 0;

    while (i < num_bits) {
    
        // we get anything left in the buffer first
        if (b_buffer->size) {
            int bit = binaryio_buffer_pop(b_buffer);
            if (bit != -1) {
                result = (result << 1) | bit;
                i++; 
            }
        }

        // now we get a byte from the specified stream
        else {
            unsigned char byte;
            if (fread(&byte, 1, 1, stream) < 1) { // we've reached EOF
                *data = result; 
                return -1; 
            }
        
            // we add bits from the byte we read to the result
            // we stop whenever we've fully consumed the byte
            // or we've read as many bits as we need to, whichever comes first
            int cur_bits_read = 0;
            while (cur_bits_read < CHAR_BIT && i < num_bits) {
                int bit = (byte >> (CHAR_BIT - cur_bits_read - 1)) & 1;
                result = (result << 1) | bit;
                i++; 
                cur_bits_read++; 
            }

            // we have left over bits, which we store in the bin buffer
            while (i >= num_bits && cur_bits_read < CHAR_BIT) {
                int bit = (byte >> (CHAR_BIT - cur_bits_read - 1)) & 1;
                binaryio_buffer_push(b_buffer, bit);
                cur_bits_read++;
            }

        }

    }

    *data = result;
    return 1; 


}
