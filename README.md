# Lossless File Compressor 

C Implementation of a lossless file compression suite, using the Lempel-Ziv-Welch (LZW) algorithm, 
with on-par or better compression performance than the native UNIX `compress` utility. 

Benchmarks (See [Testing](##Testing)) suggest that for intermediate code sizes (`MAXBITS` = 11 to 13),
this implementation achieves about 7-12% higher compression ratios at the expense of a significant increase in compression time.

For smaller or much larger code sizes, the compression ratios are roughly on par as `compress`, 
but still have a significant compute time penalty. 
To guard against this time penalty, string table pruning only occurs for `MAXBITS > 10`, which reduces computation time in half.
Based on this testing, the recommended (and default) setting for `MAXBITS` is 12.

##  Lempel-Ziv-Welch (LZW) Algorithm

The LZW algorithm is a universal lossless compression algorithm that with very high throughput. 
It is used by the UNIX utility `compress` and `uncompress` as well as in GIFs.
The algorithm works by constructing a string table of (code, prefix, and character) triples, 
which allows a long substring to be represented using a smaller code. 
Because the string table is conducted on the fly in both compression and decompression, 
the compressed file doesn't need to contain these mappings, allowing it to achieve high compression ratios. 

## Usage

From the root of the repository, run `make` with `gcc` installed to compile the source code into the executable binaries.
```sh
./compress [-m MAXBITS] < input > output\n
./decompress < input > output
```
`MAXBITS` is the largest number of bits a code can be represented with when compressing (defaults to 12).
The string table can therefore never be larger than $2^{MAXBITS}$ entries. 
When decompressing, the `MAXBITS` flag isn't passed as the compressed file stores its value.

When the `DBG` environment variable is set to 1, compress and decompress will dump human readable
versions of the final string tables to `DBG.compress` and `DBG.decompress`, respectively. 

### Example Usage

```sh
./compress < "foo.txt" > "compressed_foo.txt"
./decompress < "compressed_foo.txt" > "foo_copy.txt"
```

## Testing

The repository includes the testing script as well as the files used for testing the compressor.
Note that the testing script requires the `ncompress` utility to be installed, 
which may or not be natively installed. 
To run the test yourself, execute the following command from the root of the repository:
```sh
./tests/test_script.sh [[-m MAXBITS] [-d] [-b]]
```
The `-d` flag sets `DBG=1` and the `-b` flag stops further tests after the first error. 

The testing files come courtesy of the testing data for [Snappy](https://github.com/google/snappy), a compressor/decompressor from Google. 
The files come from a variety of sources including the Canterbury Corpus.

