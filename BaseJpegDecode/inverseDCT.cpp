#include "mbed.h"
#include "inverseDCT.h"

#ifdef USE_IDCT_AAN
const int zigzag[64] =
          {0,
           1, 8,
          16, 9, 2,
           3,10,17,24,
          32,25,18,11, 4,
          5,12,19,26,33,40,
          48,41,34,27,20,13,6,
           7,14,21,28,35,42,49,56,
          57,50,43,36,29,22,15,
          23,30,37,44,51,58,
          59,52,45,38,31,
          39,46,53,60,
          61,54,47,
          55,62,
          63};

void inverseDCT::inputBLOCK(int mcu, int block, int scan, int value) {
    if (scan == 0) {
        for(int i = 0; i < 64; i++) {
            m_s[i] = 0;
        }
    }
    m_s[zigzag[scan]] = value;
    if (scan == 63) { // last
        idct.conv(reinterpret_cast<int8_t*>(m_s), m_s);
        outputBLOCK(mcu, block, reinterpret_cast<int8_t*>(m_s));
    }
};
#endif //USE_IDCT_AAN

#ifdef USE_IDCT_TABLE
#include "inverseDCT_table.h"
void inverseDCT::inputBLOCK(int mcu, int block, int scan, int value)
{
    if (scan == 0) {
        int t = value * 32 / 8;
        for(int i = 0; i < 64; i++) {
            m_sum[i] = t;
        }
        return;
    }

    if (value != 0) {
        for(int xy = 0; xy < 64; xy++) {
            m_sum[xy] += idct_table[scan*64+xy] * value / 16;
        }
    }
    
    if (scan == 63) {
        int8_t* result = reinterpret_cast<int8_t*>(m_sum);
        for(int i = 0; i < 64; i++) {
            int t = m_sum[i] / 32;
            if (t > 127) {
                t = 127;
            } else if (t < -128) {
                t = -128;
            }
            result[i] = t;
        }
        outputBLOCK(mcu, block, result);
    }
}
#endif //USE_IDCT_TABLE
