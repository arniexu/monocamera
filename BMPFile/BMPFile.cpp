// ==================================================== Mar 04 2015, kayeks ==
// BMPFile.cpp
// ===========================================================================
// File library for Bitmap images (*.bmp, *.dib).

#include "BMPFile.h"

const char* BMPFile::StatusString[] = {
    "Success",
    "NullFileName",
    "NoSuchFile",
    "NotABitmapFile",
    "UnsupportedFormat",
    "UnsupportedDepth",
    "AllocationFailed"
};
const char* BMPFile::FormatString[] = {
    "OS2_V1",
    "OS2_V2",
    "Windows_V3",
    "Windows_V4",
    "Windows_V5",
    "Unknown"
};

void BMPFile::readFile(FILE* fp, bool fetchData) {
    uint8_t  buf8[2];
    uint16_t buf16[2];
    uint32_t buf32[3];
    
    if (!fp) {
        status = NoSuchFile;
        return;
    }
    
    // (0 +2) Magic number ("BM")
    fread(buf8, 1, 2, fp);
    if (buf8[0] != 'B' || buf8[1] != 'M') {
        status = NotABitmapFile;
        fclose(fp);
        return;
    }
    
    // (2 +4) File size
    // (6 +4) Reserved area
    // (10 +4) Image data offset (unconfirmed)
    fread(buf32, 4, 3, fp);
    fileSize = buf32[0];
    
    // (14 +4) Header size
    fread(buf32, 4, 1, fp);
    switch (buf32[0]) {
    case 12:  // OS/2 V1 format
        format = OS2_V1;
        break;
    case 64:  // OS/2 V2 format
        format = OS2_V2;
        break;
    case 40:  // Windows V3 format
        format = Windows_V3;
        break;
    case 108:  // Windows V4 format
        format = Windows_V4;
        break;
    case 124:  // Windows V5 format
        format = Windows_V5;
        break;
    default:
        format = Unknown;
    }
    
    switch (format) {
    case OS2_V1:
        // (18 +2) Bitmap width
        // (20 +2) Bitmap height
        fread(buf16, 4, 2, fp);
        width = (uint32_t) buf16[0];
        height = (uint32_t) buf16[1];
        
        // (22 +2) Number of planes (unconfirmed)
        // (24 +2) Color depth
        fread(buf16, 2, 2, fp);
        colorDepth = buf16[1];
        switch (colorDepth) {
        case 1: case 4: case 8:
            paletteSize = paletteElemSize(format) * (1ul << colorDepth);
            break;
        case 16: case 24: case 32:  // No palette
            paletteSize = 0;
            break;
        default:
            paletteSize = 0;
            status = UnsupportedDepth;
            fclose(fp);
            return;
        }
        break;
    case Windows_V3:
        // (18 +4) Bitmap width
        // (22 +4) Bitmap height
        fread(buf32, 4, 2, fp);
        width = buf32[0];
        height = buf32[1];
        
        // (26 +2) Number of planes (unconfirmed)
        // (28 +2) Color depth
        fread(buf16, 2, 2, fp);
        colorDepth = buf16[1];
        switch (colorDepth) {
        case 1: case 4: case 8:
            paletteSize = paletteElemSize(format) * (1ul << colorDepth);
            break;
        case 16: case 24: case 32:  // No palette
            paletteSize = 0;
            break;
        default:
            paletteSize = 0;
            status = UnsupportedDepth;
            fclose(fp);
            return;
        }
        
        // (30 +4) Compression method (unconfirmed)
        // (34 +4) Bitmap data size (unconfirmed)
        // (38 +4) Horizontal resolution (unused)
        // (42 +4) Vertical resolution (unconfirmed)
        // (46 +4) Colors (unconfirmed)
        // (50 +4) Important colors (unconfirmed)
        fread(buf32, 4, 3, fp);
        fread(buf32, 4, 3, fp);
        break;
    case OS2_V2:
    case Windows_V4:
    case Windows_V5:
    case Unknown:
        status = UnsupportedFormat;
        fclose(fp);
        return;
    }
    
    // Allocate palette space if needed
    if (paletteSize) {
        palette = new uint8_t[paletteSize];
        
        // Read palette data
        fread(palette, 1, paletteSize, fp);
    }
    
    // Calculate stride / data length
    if (colorDepth == 1) {
        stride = ALIGN_BY_4((width + 7) / 8);
    } else if (colorDepth == 4) {
        stride = ALIGN_BY_4((width + 1) / 2);
    } else {
        // Color depth: 8, 16, 24, 32
        stride = ALIGN_BY_4(width * colorDepth / 8);
    }
    dataSize = stride * height;
    
    // Read image data
    if (fetchData) {
        readImageData(fp);
    }
    fclose(fp);
}

BMPFile::BMPFile(const char* filename, bool fetchData) {
    FILE* fp;
    
    status = Success;
    format = Unknown;
    fileSize = 0;
    paletteSize = 0;
    dataSize = 0;
    stride = 0;
    width = 0;
    height = 0;
    colorDepth = 0;
    palette = NULL;
    data = NULL;
    
    // Open file
    if (!filename) {
        status = NullFilename;
        return;
    }
    fp = fopen(filename, "rb");
    
    // Read file
    readFile(fp, fetchData);
}

BMPFile::BMPFile(FILE* fp, bool fetch) {
    status = Success;
    format = Unknown;
    fileSize = 0;
    paletteSize = 0;
    dataSize = 0;
    stride = 0;
    width = 0;
    height = 0;
    colorDepth = 0;
    palette = NULL;
    data = NULL;
    
    // Read file
    readFile(fp, fetch);
}

BMPFile::~BMPFile() {
    if (palette) {
        delete[] palette;
    }
    if (data) {
        delete[] data;
    }
}

void BMPFile::readImageData(FILE* fp) {
    // Allocate data space
    data = new uint8_t[dataSize];
    
    // Read bitmap data
    fread(data, 1, dataSize, fp);
}

uint32_t BMPFile::paletteElemSize(BMPFile::Format format) {
    switch (format) {
    case OS2_V1: case OS2_V2:
        // BGR888
        return 3;
    case Windows_V3: case Windows_V4: case Windows_V5:
        // BGRX8888
        return 4;
    default:
        return 0;
    }
}

int32_t BMPFile::paletteRed(uint8_t index) {
    if (!palette) {
        return -1;
    }
    switch (colorDepth) {
    case 1: case 4: case 8:
        if (index >= 1ul << colorDepth) {
            return -1;
        } else {
            return palette[paletteElemSize(format) * index + 2];
        }
    default:
        return -1;
    }
}

int32_t BMPFile::paletteGreen(uint8_t index) {
    if (!palette) {
        return -1;
    }
    switch (colorDepth) {
    case 1: case 4: case 8:
        if (index >= 1ul << colorDepth) {
            return -1;
        } else {
            return palette[paletteElemSize(format) * index + 1];
        }
    default:
        return -1;
    }
}

int32_t BMPFile::paletteBlue(uint8_t index) {
    if (!palette) {
        return -1;
    }
    switch (colorDepth) {
    case 1: case 4: case 8:
        if (index >= 1ul << colorDepth) {
            return -1;
        } else {
            return palette[paletteElemSize(format) * index];
        }
    default:
        return -1;
    }
}

int32_t BMPFile::red(uint32_t x, uint32_t y) {
    if (x >= width || y >= height) {
        return -1;
    }
    if (!data) {
        return -1;
    }
    switch (colorDepth) {
    case 1:  // Indexed from palette
        return paletteRed((data[stride * y + x / 8] >> 7 - x % 8) & 0x01);
    case 4:  // Indexed from palette
        return paletteRed((data[stride * y + x / 2] >> 4 * (1 - x % 2)) & 0x0f);
    case 8:  // Indexed from palette
        return paletteRed(data[stride * y + x]);
    case 16:  // BGR565 (bbbbbggg:gggrrrrr)
        return (data[stride * y + 2 * x + 1] & 0x1f) * 2;
    case 24:  // BGR888
        return data[stride * y + 3 * x + 2];
    case 32:  // BGRX8888
        return data[stride * y + 4 * x + 2];
    default:
        return -1;
    }
}

int32_t BMPFile::green(uint32_t x, uint32_t y) {
    if (x >= width || y >= height) {
        return -1;
    }
    if (!data) {
        return -1;
    }
    switch (colorDepth) {
    case 1:  // Indexed from palette
        return paletteGreen((data[stride * y + x / 8] >> 7 - x % 8) & 0x01);
    case 4:  // Indexed from palette
        return paletteGreen((data[stride * y + x / 2] >> 4 * (1 - x % 2)) & 0x0f);
    case 8:  // Indexed from palette
        return paletteGreen(data[stride * y + x]);
    case 16:  // BGR565 (bbbbbggg:gggrrrrr)
        return ((data[stride * y + 2 * x] & 0x07) << 3
                | data[stride * y + 2 * x + 1] >> 5) * 2;
    case 24:  // BGR888
        return data[stride * y + 3 * x + 1];
    case 32:  // BGRX8888
        return data[stride * y + 4 * x + 1];
    default:
        return -1;
    }
}

int32_t BMPFile::blue(uint32_t x, uint32_t y) {
    if (x >= width || y >= height) {
        return -1;
    }
    if (!data) {
        return -1;
    }
    switch (colorDepth) {
    case 1:  // Indexed from palette
        return paletteBlue((data[stride * y + x / 8] >> 7 - x % 8) & 0x01);
    case 4:  // Indexed from palette
        return paletteBlue((data[stride * y + x / 2] >> 4 * (1 - x % 2)) & 0x0f);
    case 8:  // Indexed from palette
        return paletteBlue(data[stride * y + x]);
    case 16:  // RGB565 (bbbbbggg:gggrrrrr)
        return (data[stride * y + 2 * x] >> 3) * 2;
    case 24:  // BGR888
        return data[stride * y + 3 * x];
    case 32:  // BGRX8888
        return data[stride * y + 4 * x];
    default:
        return -1;
    }
}
