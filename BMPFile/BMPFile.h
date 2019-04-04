// ==================================================== Mar 04 2015, kayeks ==
// BMPFile.h
// ===========================================================================
// File library for Bitmap images (*.bmp, *.dib).

#ifndef BMPFILE_H_
#define BMPFILE_H_

#include "mbed.h"

#define ALIGN_BY_4(x)  (((x) + 3) / 4 * 4)

struct BMPFile {
    /** Object status.
     *  This indicates the error type when reading/parsing file is failed. */
    enum Status {
        Success              /** Read successfully. (no errors) */
        , NullFilename       /** File name string is missing. */
        , NoSuchFile         /** File is missing or cannot be opened. */
        , NotABitmapFile     /** File is not a bitmap image. (wrong magic number) */ 
        , UnsupportedFormat  /** Unsupported bitmap format type. */
        , UnsupportedDepth   /** Unsupported color depth configuration.
                                 This library does not support any indexed color depth (1/4/8 bpp). */
        , AllocationFailed   /** Data space allocation failed. */
    };
    
    /** Bitmap format type. */
    enum Format {
        OS2_V1        /** OS/2 bitmap version 1. header size is 12 bytes. */
        , OS2_V2      /** OS/2 bitmap version 2. header size is 64 bytes. */
        , Windows_V3  /** Windows bitmap version 3. header size is 40 bytes. */
        , Windows_V4  /** Windows bitmap version 4. header size is 108 bytes. */
        , Windows_V5  /** Windows bitmap version 5. header size is 124 bytes. */
        , Unknown     /** Unknown bitmap format type. */
    };
    static const char* StatusString[];
    static const char* FormatString[];

    /** The object status. */
    Status   status;
    
    /** The bitmap format type. */
    Format   format;
    
    /** The image file size in bytes, including its header part. */
    uint32_t fileSize;

    /** The image palette size in bytes. */
    uint32_t paletteSize;
    
    /** The image data part size in bytes. */
    uint32_t dataSize;
    
    /** The image stride (size per line) in bytes. */
    uint32_t stride;
    
    /** The image width in pixels. */
    uint32_t width;
    
    /** The image height in pixels. */
    uint32_t height;
    
    /** The color depth of the image in bit-per-pixel. */
    uint16_t colorDepth;
    
    /** The indexed color palette of the image (raw array). */
    uint8_t* palette;
    
    /** The data part of image (raw array). */
    uint8_t* data;
    
    /** Constructor of struct BMPFile.
     *  @param filename   Input file name string.
     *  @param fetchData  Fetch image data on construct. Default value: true.
     */
    BMPFile(const char* filename, bool fetchData=true);
    /** Constructor of struct BMPFile.
     *  @param fp         Input file pointer.
     *  @param fetchData  Fetch image data on construct. Default value: true.
     */
    BMPFile(FILE* fp, bool fetchData=true);
    
    /** Destructor of struct BMPFile. */
    ~BMPFile();
    
    /** Get red value of specified pixel.
     *  @param x  X-coordinate of the pixel.
     *  @param y  Y-coordinate of the pixel.
     */
    int32_t red(uint32_t x, uint32_t y);

    /** Get green value of specified pixel.
     *  This method returns -1 for outranged pixels.
     *  @param x  X-coordinate of the pixel.
     *  @param y  Y-coordinate of the pixel.
     */
    int32_t green(uint32_t x, uint32_t y);

    /** Get blue value of specified pixel.
     *  This method returns -1 for outranged pixels.
     *  @param x  X-coordinate of the pixel.
     *  @param y  Y-coordinate of the pixel.
     */
    int32_t blue(uint32_t x, uint32_t y);
    
    /** Get red value from palette.
     *  This method returns -1 for non-indexed image or outranged indexes.
     *  @param index  Palette color index.
     */
    int32_t paletteRed(uint8_t index);

    /** Get green value from palette.
     *  This method returns -1 for non-indexed image or outranged indexes.
     *  @param index  Palette color index.
     */
    int32_t paletteGreen(uint8_t index);

    /** Get blue value from palette.
     *  This method returns -1 for non-indexed image or outranged indexes.
     *  @param index  Palette color index.
     */
    int32_t paletteBlue(uint8_t index);

private:
    void readFile(FILE* fp, bool fetchData);
    void readImageData(FILE* fp);
    static uint32_t paletteElemSize(Format format);
};

#endif
