// BaseJpegDecode.h 2013/1/27
#ifndef BASE_JPEG_DECODE_H
#define BASE_JPEG_DECODE_H
#include "bjd_config.h"
#include "HuffmanDecode.h"
#include "BitPattern.h"

class BaseJpegDecode {
public:
    BaseJpegDecode();
    void clear();
    void input(uint8_t c);
    void input(uint8_t* buf, int len);
    virtual void outputDC(int mcu, int block, int value) = 0;
    virtual void outputAC(int mcu, int block, int scan, int value) = 0;
    virtual void outputMARK(uint8_t c) = 0;
    uint8_t* qt[2];
    int width;
    int height;
    int yblock;
#ifdef JPEG_USE_REPORT_CODE
    int report_scan_count;
    int report_scan_dc_count;
    int report_scan_ac_count;
#endif //JPEG_USE_REPORT
private:
    void inputDQT(uint8_t c, int pos, int len);
    void inputSOF(uint8_t c, int pos, int len);
    void inputScan(uint8_t c);
    void restart();
    uint8_t m_seq;
    int m_mcu;
    int m_block;
    int m_scan;
    Huff* m_huff;
    int m_pre_DC_value[3];
    BitPattern m_bitpat;
    uint8_t m_mark;
    HuffmanDecode* pHD;
    int m_seg_pos;
    int m_seg_len;
    int m_param1;
};
#endif // BASE_JPEG_DECODE_H
