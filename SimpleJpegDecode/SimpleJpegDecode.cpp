#include "SimpleJpegDecode.h"

#define DBG(...) do{fprintf(stderr,"[%s@%d] ",__PRETTY_FUNCTION__,__LINE__);fprintf(stderr,__VA_ARGS__);} while(0);
#define ASSERT(A) while(!(A)){fprintf(stderr,"\n\n%s@%d %s ASSERT!\n\n",__PRETTY_FUNCTION__,__LINE__,#A);exit(1);};


#define _1_4020 45
#define _0_3441 11
#define _0_7139 23
#define _1_7718 57
#define _0_0012 0

int adjust(int r) {
    if (r >= 0) {
        if (r <= 255) {
            return r;
        } else {
            return 255;
        }
    } else {
        return 0;
    }
}
    
void convYUVtoRGB(uint8_t rgb[], int y, int u, int v)
{
    rgb[0] = adjust((y*32             + v*_1_4020)/32 + 128);
    rgb[1] = adjust((y*32 - u*_0_3441 - v*_0_7139)/32 + 128);
    rgb[2] = adjust((y*32 + u*_1_7718 - v*_0_0012)/32 + 128);
}

SimpleJpegDecode::SimpleJpegDecode(uint8_t output_mode)
{
    m_output_mode = output_mode;
    clearOnResult();
}


void SimpleJpegDecode::output(int mcu, int block, int scan, int value)
{
    int sc = (block < yblock) ? 0 : 1;
    inputBLOCK(mcu, block, scan, value * qt[sc][scan]);
}

void SimpleJpegDecode::outputDC(int mcu, int block, int value)
{
    output(mcu, block, 0, value);
    DC_count++;
}

void SimpleJpegDecode::outputAC(int mcu, int block, int scan, int value)
{
    output(mcu, block, scan, value);
    AC_count++;
}

void SimpleJpegDecode::outputMARK(uint8_t c)
{
}

void SimpleJpegDecode::format_YUV(int mcu, int block, int8_t* values)
{
    if (block < yblock+1) {
        memcpy(m_block_data[block], values, 64);
        return;
    }
    int mcu_x_count = (width+15)/16;
    int mcu_x = mcu % mcu_x_count;
    int mcu_y = mcu / mcu_x_count;
    uint8_t yuv[3];
    if (yblock == 2) {
        for(int y = 0; y < 8; y++) {
            for(int x = 0; x < 16; x++) {
                yuv[0] = m_block_data[x/8][y*8+x%8] + 128;
                yuv[1] = m_block_data[2][y*8+x/2] + 128;
                yuv[2] = values[y*8+x/2] + 128;
                onResult(mcu_x * 16 + x, mcu_y * 8 + y, yuv);
            }
        }
    } else if (yblock == 4) {
        for(int y = 0; y < 16; y++) {
            for(int x = 0; x < 16; x++) {
                yuv[0] = m_block_data[(y/8)*2+x/8][(y%8)*8+x%8] + 128;
                yuv[1] = m_block_data[4][(y/2)*8+x/2] + 128;
                yuv[2] = values[(y/2)*8+x/2] + 128;
                onResult(mcu_x * 16 + x, mcu_y * 16 + y, yuv);
            }
        }
    } else {
        ASSERT(yblock == 2 || yblock == 4);
    }    
}

void SimpleJpegDecode::format_RGB24(int mcu, int block, int8_t* values)
{
    if (block < yblock+1) {
        memcpy(m_block_data[block], values, 64);
        return;
    }
    int mcu_x_count = (width+15)/16;
    int mcu_x = mcu % mcu_x_count;
    int mcu_y = mcu / mcu_x_count;
    uint8_t rgb[3];
    if (yblock == 2) {
        for(int y = 0; y < 8; y++) {
            for(int x = 0; x < 16; x++) {
                int8_t yuv_y = m_block_data[x/8][y*8+x%8];
                int8_t yuv_u = m_block_data[2][y*8+x/2];
                int8_t yuv_v = values[y*8+x/2];
                convYUVtoRGB(rgb, yuv_y, yuv_u, yuv_v);
                onResult(mcu_x * 16 + x, mcu_y * 8 + y, rgb);
            }
        }
    } else if (yblock == 4) {
        for(int y = 0; y < 16; y++) {
            for(int x = 0; x < 16; x++) {
                int8_t yuv_y = m_block_data[(y/8)*2+x/8][(y%8)*8+x%8];
                int8_t yuv_u = m_block_data[4][(y/2)*8+x/2];
                int8_t yuv_v = values[(y/2)*8+x/2];
                convYUVtoRGB(rgb, yuv_y, yuv_u, yuv_v);
                onResult(mcu_x * 16 + x, mcu_y * 16 + y, rgb);
            }
        }
    } else {
        ASSERT(yblock == 2 || yblock == 4);
    }    
}

void SimpleJpegDecode::outputBLOCK(int mcu, int block, int8_t* values)
{
    BLOCK_count++;
    if (m_output_mode == YUV) {
        format_YUV(mcu, block, values);
    } else if (m_output_mode == RGB24) {
        format_RGB24(mcu, block, values);
    } else {
        ASSERT(m_output_mode == YUV || m_output_mode == RGB24);
    }    
}

void SimpleJpegDecode::onResult(int x, int y, uint8_t* yuv)
{
  if(m_pCbItem && m_pCbMeth)
    (m_pCbItem->*m_pCbMeth)(x, y, yuv);
  else if(m_pCb)
    m_pCb(x, y, yuv);
}

void SimpleJpegDecode::setOnResult( void (*pMethod)(int, int, uint8_t*) )
{
  m_pCb = pMethod;
  m_pCbItem = NULL;
  m_pCbMeth = NULL;
}

void SimpleJpegDecode::clearOnResult()
{
  m_pCb = NULL;
  m_pCbItem = NULL;
  m_pCbMeth = NULL;
}
