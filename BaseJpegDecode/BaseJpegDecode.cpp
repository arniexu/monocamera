// BaseJpegDecode.cpp 2013/1/27
#include "mbed.h"
#include "BaseJpegDecode.h"

#if 0
#define DBG(...) do{fprintf(stderr,"[%s@%d] ",__PRETTY_FUNCTION__,__LINE__);fprintf(stderr,__VA_ARGS__);} while(0);
#define DBG_WAIT(A) wait_ms(A)
#else
#define DBG(...)
#define DBG_WAIT(A)
#endif

#if 0
#define DBG_ASSERT(A) while(!(A)){fprintf(stderr,"\n\n%s@%d %s ASSERT!\n\n",__PRETTY_FUNCTION__,__LINE__,#A);exit(1);};
#else
#define DBG_ASSERT(A) 
#endif

#ifdef JPEG_USE_REPORT_CODE
#define REPORT_CODE(A) (A)
#else
#define REPORT_CODE(A)
#endif

#define MARK_SOF0 0xc0
#define MARK_DHT  0xc4
#define MARK_RST0 0xd0
#define MARK_RST7 0xd7
#define MARK_SOI  0xd8
#define MARK_EOI  0xd9
#define MARK_SOS  0xda
#define MARK_DQT  0xdb
#define MARK_DRI  0xdd
#define MARK_APP  0xe0

#define SEQ_INIT     0
#define SEQ_SOI      1
#define SEQ_FRAME    2
#define SEQ_MARK     3
#define SEQ_SEG_LEN  4
#define SEQ_SEG_LEN2 5
#define SEQ_SEG_BODY 6
#define SEQ_SOS      7
#define SEQ_SOS2     8

#define HT_DC 0
#define HT_AC 1

BaseJpegDecode::BaseJpegDecode()
{
    yblock = JPEG_MCU_YBLOCK; // 2 or 4
    clear();
    pHD = new HuffmanDecode;
    DBG_ASSERT(pHD);
    if (pHD == NULL) {
        return;
    }
    qt[0] = new uint8_t[64];
    qt[1] = new uint8_t[64];
    DBG_ASSERT(qt[0]);
    DBG_ASSERT(qt[1]);
}

void BaseJpegDecode::clear()
{
    m_seq = SEQ_INIT;
}

void BaseJpegDecode::input(uint8_t* buf, int len)
{
    for(int i = 0; i < len; i++) {
        input(buf[i]);
    }
}

void BaseJpegDecode::restart()
{
    m_block = 0;
    m_scan = 0;
    m_pre_DC_value[0] = 0;
    m_pre_DC_value[1] = 0;
    m_pre_DC_value[2] = 0;
    m_bitpat.clear();
    m_huff = NULL;
}

void BaseJpegDecode::inputDQT(uint8_t c, int pos, int len)
{
    if (pos == 0 || pos == 65) {
        m_param1 = c;
        DBG_ASSERT(m_param1 == 0 || m_param1 == 1);
    } else {
        if (m_param1 == 0 || m_param1 == 1) { 
            if(qt[m_param1]) {
                qt[m_param1][(pos%65)-1] = c;
            }
        }
    }
}

void BaseJpegDecode::inputSOF(uint8_t c, int pos, int len)
{
    switch(pos) {
        case 1:
            height = (height&0x00ff) | (c<<8);
            break;
        case 2:
            height = (height&0xff00) | c;
            break;
        case 3:
            width = (width&0x00ff) | (c<<8);
            break;
        case 4:
            width = (width&0xff00) | c;
            break;
        case 7:
            if (c == 0x22) {
                yblock = 4;
            } else if (c == 0x21) {
                yblock = 2;
            } else {
                DBG_ASSERT(c == 0x22 || c == 0x21);
            }
            break;
    }
}

void BaseJpegDecode::input(uint8_t c)
{
    switch(m_seq) {
        case SEQ_INIT:
            if (c == 0xff) {
                m_seq = SEQ_SOI;
            }
            break;
        case SEQ_SOI:
            if (c == MARK_SOI) {
                outputMARK(c);
                m_seq = SEQ_FRAME;
            } else {
                m_seq = SEQ_INIT;
            }
            break;
        case SEQ_FRAME:
            if (c == 0xff) {
                m_seq = SEQ_MARK;
            } else {
                m_seq = SEQ_INIT;
            }
            break;
        case SEQ_MARK:
            outputMARK(c);
            if (c == MARK_SOI) {
                m_seq = SEQ_FRAME;
                break;
            } else if (c == MARK_EOI || c == 0x00) {
                m_seq = SEQ_INIT;
                break;
            }
            m_mark = c;
            m_seq = SEQ_SEG_LEN;
            break;
        case SEQ_SEG_LEN:
            m_seg_len = c;
            m_seq = SEQ_SEG_LEN2;
            break;
        case SEQ_SEG_LEN2:
            m_seg_len <<= 8;
            m_seg_len |= c;
            m_seg_len -= 2;
            m_seg_pos = 0;
            m_seq = SEQ_SEG_BODY;
            break;
        case SEQ_SEG_BODY:
            if (m_mark == MARK_DQT) {
                inputDQT(c, m_seg_pos, m_seg_len);
            } else if (m_mark == MARK_SOF0) {
                inputSOF(c, m_seg_pos, m_seg_len);
            }
            if (++m_seg_pos < m_seg_len) {
                break;
            }
            if (m_mark == MARK_SOS) {
                m_seq = SEQ_SOS;
                m_mcu = 0;
                restart();
                break;
            }
            m_seq = SEQ_FRAME;
            break;
        case SEQ_SOS:
            if (c == 0xff) {
                m_seq = SEQ_SOS2;
                break;
            }
            inputScan(c);
            break;
        case SEQ_SOS2:
            if (c == 0x00) {
                inputScan(0xff);
                m_seq = SEQ_SOS;
                break;
            } else if (c >= MARK_RST0 && c <= MARK_RST7) {
                restart();
                m_seq = SEQ_SOS;
                break;
            }
            outputMARK(c);
            m_seq = SEQ_INIT;
            break;
        default:
            break;
    }
}

void BaseJpegDecode::inputScan(uint8_t c)
{
    m_bitpat += c;
    while(m_bitpat.size() > 0) {
        int tc = (m_scan == 0) ? HT_DC : HT_AC;
        int th = (m_block < yblock) ? 0 : 1;
        DBG("%d %d %08x %d\n", tc, th, m_bitpat.peek(32), m_bitpat.size());
        if (m_huff == NULL) {
            m_huff = pHD->Lookup(tc, th, &m_bitpat);
            if (m_huff == NULL) {
                break;
            }
            m_bitpat.get(m_huff->code_size); // skip code
        }
        if (m_huff->value_size > m_bitpat.size()) {
            break;
        }
        DBG("%d %d %d %02x\n", m_huff->run, m_huff->value_size, m_huff->code_size, m_huff->code);
        int value = pHD->getValue(m_huff, &m_bitpat);
        if (tc == HT_DC) {
            int sc = 0; // Y
            if (m_block == yblock) {
                sc = 1; // Cb
            } else if (m_block == (yblock+1)) {
                sc = 2; // Cr
            }
            value += m_pre_DC_value[sc];
            outputDC(m_mcu, m_block, value);
            m_pre_DC_value[sc] = value;
            m_scan++;
            REPORT_CODE(report_scan_dc_count++);
        } else { // AC
            if (m_huff->value_size == 0 && m_huff->run == 0) { // EOB
                DBG("AC EOB\n");
                outputAC(m_mcu, m_block, 63, 0);
                m_scan = 64;
            } else {
                for(int i = 0; i < m_huff->run; i++) {
                    //outputAC(m_mcu, m_block, m_scan, 0);
                    m_scan++;
                }
                outputAC(m_mcu, m_block, m_scan, value);
                m_scan++;
            }
            if (m_scan >= 64) {
                m_scan = 0;
                if (++m_block >= (yblock+2)) {
                    m_block = 0;
                    m_mcu++;
                }
            }
            REPORT_CODE(report_scan_ac_count++);
        }
        m_huff = NULL;
        REPORT_CODE(report_scan_count++);
    }
}
