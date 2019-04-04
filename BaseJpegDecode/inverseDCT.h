// inverseDCT.h 2013/1/28
#ifndef INVERSE_DCT_H
#define INVERSE_DCT_H

#define USE_IDCT_AAN
//#define USE_IDCT_TABLE

#ifdef USE_IDCT_AAN
#include "aanIDCT.h"
class inverseDCT {
public:
    void inputBLOCK(int mcu, int block, int scan, int value);
    virtual void outputBLOCK(int mcu, int block, int8_t * values) = 0;
private:
    int16_t m_s[64];
    aanIDCT idct;
};
#endif // USE_IDCT_AAN

#ifdef USE_IDCT_TABLE
class inverseDCT {
public:
    void inputBLOCK(int mcu, int block, int scan, int value);
    virtual void outputBLOCK(int mcu, int block, int8_t* values) = 0;
private:
    int16_t m_sum[64];
};
#endif // USE_IDCT_TABLE
 
#endif // INVERSE_DCT_H
