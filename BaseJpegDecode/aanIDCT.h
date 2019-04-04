#ifndef AAN_IDCT_H
#define AAN_IDCT_H

class aanIDCT {
public:
    void conv(int8_t output[], int16_t input[]);
private:
    int ws[64];
};

#endif // ANN_IDCT_H
