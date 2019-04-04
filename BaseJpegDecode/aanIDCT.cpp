// aanIDCT.cpp 2013/2/1
// based: http://www.ijg.org/ libjpeg(jpeg-8d)jidctfst.c jidctflt.c jidctmgr.c
#include "mbed.h"
#include "aanIDCT.h"

#define DCTSIZE 8
#define DCTSIZE2 64
#define CONST_BITS 8
#define PASS1_BITS 2
#define SCALE_BITS 14
#define LOG2_CONST 3
#define FIX_1_082392200 277
#define FIX_1_414213562 362
#define FIX_1_847759065 473
#define FIX_2_613125930 668
const uint16_t aanscales[] = {
16384, 22725, 21406, 19265, 16384, 12872,  8866,  4520,
22725, 31520, 29691, 26722, 22725, 17855, 12298,  6269,
21406, 29691, 27969, 25171, 21406, 16819, 11585,  5906,
19265, 26722, 25171, 22653, 19265, 15136, 10426,  5315,
16384, 22725, 21406, 19265, 16384, 12872,  8866,  4520,
12872, 17855, 16819, 15136, 12872, 10114,  6966,  3551,
 8866, 12298, 11585, 10426,  8866,  6966,  4798,  2446,
 4520,  6269,  5906,  5315,  4520,  3551,  2446,  1247,
};

#if 0
inline int DESCALE(int x, int n) {
    return (x / (1<<(n)));
}
#else
#define DESCALE(x, n) ((x)/(1<<(n)))
#endif

int DEQUANTIZE(int coef, int quantval) {
    return DESCALE(coef * quantval, SCALE_BITS-PASS1_BITS);
}

#if 1
int MULTIPLY(int a, int b) {
    return DESCALE(a * b, CONST_BITS);
}
#else
#define MULTIPLY(a, b) (((a)*(b))/(1<<CONST_BITS))
#endif

int IDESCALE(int x) {
    return DESCALE(x, PASS1_BITS+LOG2_CONST);
}

#if 1
int8_t range_limit(int val) {
    if (val < -128) {
        return -128;
    } else if (val > 127) {
        return 127;
    }
    return val;
}
#else
inline int8_t range_limit(int val) {
    if (val < -128) {
        return -128;
    } else if (val > 127) {
        return 127;
    }
    return val;
}
#endif

void aanIDCT::conv(int8_t output[], int16_t input[])
{
    uint16_t* quant = (uint16_t*)aanscales;
    for(int pos = 0; pos < DCTSIZE; pos++) {
        if (input[pos+DCTSIZE*1] == 0 && input[pos+DCTSIZE*2] == 0 &&
               input[pos+DCTSIZE*3] == 0 && input[pos+DCTSIZE*4] == 0 &&
               input[pos+DCTSIZE*5] == 0 && input[pos+DCTSIZE*6] == 0 &&
               input[pos+DCTSIZE*7] == 0) {
            int dcval = DEQUANTIZE(input[pos+DCTSIZE*0], quant[pos+DCTSIZE*0]);
            for(int y = 0; y < DCTSIZE; y++) {
                ws[pos+DCTSIZE*y] = dcval;
            }
            continue;
        }
        // Even part
        int tmp0 = DEQUANTIZE(input[pos+DCTSIZE*0], quant[pos+DCTSIZE*0]);
        int tmp1 = DEQUANTIZE(input[pos+DCTSIZE*2], quant[pos+DCTSIZE*2]);
        int tmp2 = DEQUANTIZE(input[pos+DCTSIZE*4], quant[pos+DCTSIZE*4]);
        int tmp3 = DEQUANTIZE(input[pos+DCTSIZE*6], quant[pos+DCTSIZE*6]);

        int tmp10 = tmp0 + tmp2;    // phase 3
        int tmp11 = tmp0 - tmp2;

        int tmp13 = tmp1 + tmp3;    // phases 5-3
        int tmp12 = MULTIPLY(tmp1 - tmp3, FIX_1_414213562) - tmp13; // 2*c4

        tmp0 = tmp10 + tmp13;   // phase 2
        tmp3 = tmp10 - tmp13;
        tmp1 = tmp11 + tmp12;
        tmp2 = tmp11 - tmp12;

        // Odd part
        int tmp4 = DEQUANTIZE(input[pos+DCTSIZE*1], quant[pos+DCTSIZE*1]);
        int tmp5 = DEQUANTIZE(input[pos+DCTSIZE*3], quant[pos+DCTSIZE*3]);
        int tmp6 = DEQUANTIZE(input[pos+DCTSIZE*5], quant[pos+DCTSIZE*5]);
        int tmp7 = DEQUANTIZE(input[pos+DCTSIZE*7], quant[pos+DCTSIZE*7]);

        int z13 = tmp6 + tmp5;  // phase 6
        int z10 = tmp6 - tmp5;  
        int z11 = tmp4 + tmp7;
        int z12 = tmp4 - tmp7;

        tmp7 = z11 + z13;       // phase 5
        tmp11 = MULTIPLY(z11 - z13, FIX_1_414213562);   // 2*c4

        int z5 = MULTIPLY(z10 + z12, FIX_1_847759065);  // 2*c2
        tmp10 = MULTIPLY(z12, - FIX_1_082392200) + z5;  // 2*(c2-c6)
        tmp12 = MULTIPLY(z10, - FIX_2_613125930) + z5;  // -2*(c2+c6)

        tmp6 = tmp12 - tmp7;    // phase 2
        tmp5 = tmp11 - tmp6;
        tmp4 = tmp10 - tmp5;

        ws[pos+DCTSIZE*0] = tmp0 + tmp7;
        ws[pos+DCTSIZE*7] = tmp0 - tmp7;
        ws[pos+DCTSIZE*1] = tmp1 + tmp6;
        ws[pos+DCTSIZE*6] = tmp1 - tmp6;
        ws[pos+DCTSIZE*2] = tmp2 + tmp5;
        ws[pos+DCTSIZE*5] = tmp2 - tmp5;
        ws[pos+DCTSIZE*3] = tmp3 + tmp4;
        ws[pos+DCTSIZE*4] = tmp3 - tmp4;
    }
    
    for(int pos = 0; pos < DCTSIZE2; pos += DCTSIZE) {
        if (ws[pos+1] == 0 && ws[pos+2] == 0 && ws[pos+3] == 0 &&
               ws[pos+4] == 0 && ws[pos+5] == 0 && ws[pos+6] == 0 &&
               ws[pos+7] == 0) {
            int dcval = ws[pos+0];
            for(int x = 0; x < DCTSIZE; x++) {
                output[pos+x] = range_limit(IDESCALE(dcval));
            }
            continue;
        }
        // Even part
        int tmp10 = ws[pos+0] + ws[pos+4];
        int tmp11 = ws[pos+0] - ws[pos+4];

        int tmp13 = ws[pos+2] + ws[pos+6];
        int tmp12 = MULTIPLY(ws[pos+2] - ws[pos+6], FIX_1_414213562) - tmp13;

        int tmp0 = tmp10 + tmp13;
        int tmp3 = tmp10 - tmp13;
        int tmp1 = tmp11 + tmp12;
        int tmp2 = tmp11 - tmp12;

        // Odd part
        int z13 = ws[pos+5] + ws[pos+3];
        int z10 = ws[pos+5] - ws[pos+3];
        int z11 = ws[pos+1] + ws[pos+7];
        int z12 = ws[pos+1] - ws[pos+7];

        int tmp7 = z11 + z13;       // phase 5
        tmp11 = MULTIPLY(z11 - z13, FIX_1_414213562);   // 2*c4

        int z5 = MULTIPLY(z10 + z12, FIX_1_847759065);  // 2*c2
        tmp10 = MULTIPLY(z12, - FIX_1_082392200) + z5;  // 2*(c2-c6)
        tmp12 = MULTIPLY(z10, - FIX_2_613125930) + z5;  // -2*(c2+c6)

        int tmp6 = tmp12 - tmp7;    // phase 2
        int tmp5 = tmp11 - tmp6;
        int tmp4 = tmp10 - tmp5;

        output[pos+0] = range_limit(IDESCALE(tmp0 + tmp7));
        output[pos+7] = range_limit(IDESCALE(tmp0 - tmp7));
        output[pos+1] = range_limit(IDESCALE(tmp1 + tmp6));
        output[pos+6] = range_limit(IDESCALE(tmp1 - tmp6));
        output[pos+2] = range_limit(IDESCALE(tmp2 + tmp5));
        output[pos+5] = range_limit(IDESCALE(tmp2 - tmp5));
        output[pos+3] = range_limit(IDESCALE(tmp3 + tmp4));
        output[pos+4] = range_limit(IDESCALE(tmp3 - tmp4));
    }
}
