#ifndef COMPRESS
#define COMPRESS
/*
Compresses a stream passed into stdin using the Lempel-Ziv-Welch (LZW) algorithm.
Args:
    `int max_bits`: the number of bits to represent the largest entry in the string table.
    This has the effect of setting the maximum size of the string table to be 2^`max_bits`.
*/
void compress(int max_bits);

#endif