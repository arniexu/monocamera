#include "BitPattern.h"

BitPattern::BitPattern()
{
    clear();
}

BitPattern::BitPattern(uint32_t value, int size)
{
    clear();
    put(value, size);
}

BitPattern::BitPattern(const char* s)
{
    clear();
    operator+=(s);
}

void BitPattern::clear()
{
    m_size = 0;
    m_pat = 0;
}

void BitPattern::operator=(const char* s)
{
    clear();
    operator+=(s);
}

void BitPattern::operator+=(uint8_t c)
{
    put(c);
}

void BitPattern::operator+=(const char* s)
{
    for(int i = 0; i < 32 && s[i]; i++) {
        char c = s[i];
        put(c-'0', 1);
    }
}

int BitPattern::operator [](int index)
{
    uint32_t mask = 0x80000000;
    for(int i = 0; i < index; i++) {
        mask>>=1;
    }
    if (m_pat & mask) {
        return 1;
    }
    return 0;
}

void BitPattern::put(uint32_t value, int size)
{
    m_pat <<= size;
    m_pat |= value;
    m_size += size;
}

uint32_t BitPattern::peek(int size)
{
    return m_pat >> (m_size - size);
}

uint32_t BitPattern::get(int size)
{
    uint32_t r = peek(size);
    m_size -= size;
    m_pat &= (1<<m_size)-1;
    return r;
}

int BitPattern::size()
{
    return m_size;
}

bool BitPattern::match(BitPattern& b)
{
    if (b.m_size > m_size) {
        return false;
    }
    return peek(b.size()) == b.peek(b.size());
}

bool BitPattern::match(const char* s)
{
    BitPattern a = s;
    return match(a);
}

bool BitPattern::operator ==(BitPattern& b)
{
    if (b.m_size == m_size) {
        if (b.m_pat == m_pat) {
            return true;
        }
    }
    return false;
}
