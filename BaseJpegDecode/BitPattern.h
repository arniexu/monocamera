#ifndef BIT_PATTERN_H
#define BIT_PATTERN_H
#include "mbed.h"

class BitPattern {
public:
    BitPattern();
    BitPattern(uint32_t value, int size);
    BitPattern(const char* s);
    void clear();
    void operator +=(uint8_t c);
    int operator [](int index);
    void put(uint32_t value, int size = 8);
    uint32_t peek(int size = 8);
    uint32_t get(int size = 8);
    int size();
    bool match(BitPattern& b);
    bool match(const char* s);
    void operator =(const char* s);
    void operator +=(const char* s);
    bool operator ==(BitPattern& b);
private:
    uint32_t m_pat;
    int m_size;
};
#endif // BIT_PATTERN_H
