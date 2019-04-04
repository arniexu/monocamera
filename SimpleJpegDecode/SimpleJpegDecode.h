#ifndef SIMPLE_JPEG_DECODE_H
#define SIMPLE_JPEG_DECODE_H

#include "BaseJpegDecode.h"
#include "inverseDCT.h"

#define YUV   0
#define RGB24 1

class SimpleJpegDecode : public BaseJpegDecode, public inverseDCT {
public:
    SimpleJpegDecode(uint8_t output_mode=RGB24);

    void format_YUV(int mcu, int block, int8_t* values);
    void format_RGB24(int mcu, int block, int8_t* values);

    void output(int mcu, int block, int scan, int value);
    virtual void outputDC(int mcu, int block, int value);
    virtual void outputAC(int mcu, int block, int scan, int value);
    virtual void outputMARK(uint8_t c);
    virtual void outputBLOCK(int muc, int block, int8_t* values); // iDCT

    int8_t m_block_data[5][64];
    int DC_count;
    int AC_count;
    int BLOCK_count;

    ///Setups the result callback
    /**
     @param pMethod : callback function
     */
    void setOnResult( void (*pMethod)(int, int, uint8_t*) );
  
    ///Setups the result callback
    /**
    @param pItem : instance of class on which to execute the callback method
    @param pMethod : callback method
    */
    class CDummy;
    template<class T> 
    void setOnResult( T* pItem, void (T::*pMethod)(int, int, uint8_t*) )
    {
        m_pCb = NULL;
        m_pCbItem = (CDummy*) pItem;
        m_pCbMeth = (void (CDummy::*)(int, int, uint8_t*)) pMethod;
    }
    void clearOnResult();
protected:
    void onResult(int x, int y, uint8_t* yuv);
    CDummy* m_pCbItem;
    void (CDummy::*m_pCbMeth)(int, int, uint8_t*);
    void (*m_pCb)(int, int, uint8_t*);
    uint8_t m_output_mode;
};

#endif // SIMPLE_JPEG_DECODE_H
