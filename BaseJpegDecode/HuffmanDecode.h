#ifndef HUFFMAN_DECODE_H
#define HUFFMAN_DECODE_H
#include "BitPattern.h"

typedef struct sthuff {
    int8_t run;
    int8_t value_size;
    int8_t code_size;
    uint16_t code;
} Huff;

class HuffmanDecode {
public:
    Huff* Lookup(int tc, int th, BitPattern* bitpat);
    int getValue(Huff* huff, BitPattern* bitpat);
};

#endif // HUFFMAN_DECODE_H

