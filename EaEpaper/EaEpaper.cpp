
#include "EaEpaper.h"

#define EA_IMG_BUF_SZ (5808) // 264 x 176 / 8 = 5808

#define LM75A_ADDRESS (0x92) // 0x49 << 1
#define LM75A_CMD_TEMP 0x00

static uint8_t _oldImage[EA_IMG_BUF_SZ];
static uint8_t _newImage[EA_IMG_BUF_SZ];

EaEpaper::EaEpaper(PinName p_on, PinName border, PinName discharge,  PinName reset, PinName busy, PinName cs,
                   PinName pwm, 
                   PinName mosi, PinName miso, PinName sck, 
                   PinName sda, PinName scl, 
                   const char* name):
    epd_(EPD_2_7,
         p_on,  // panel_on_pin
         border, // border_pin
         discharge, // discharge_pin
         pwm, // pwm_pin
         reset, // reset_pin
         busy, // busy_pin
         cs,  // chip_select_pin
         mosi,  // mosi
         miso,  // miso
         sck),  // sck
    i2c_(sda, scl),
    GraphicsDisplay(name)
{
    draw_mode = NORMAL;
    memset(_oldImage, 0, EA_IMG_BUF_SZ);
    memset(_newImage, 0, EA_IMG_BUF_SZ);
}


int EaEpaper::width()
{
    return 264;
}

int EaEpaper::height()
{
    return 176;
}

int EaEpaper::cwidth(int c)
{
	int size = getCharacterSize(c);
	if(size == 22)
		return 8;
	else if(size == 44)
    	return 10;
	else
		return -1;
}

int EaEpaper::cheight()
{
    return 22;
}

// erase pixel after power up
void EaEpaper::clear()
{
    epd_.begin();
    epd_.setFactor(readTemperature()/100);
    epd_.clear();
    epd_.end();

    memset(_oldImage, 0, EA_IMG_BUF_SZ);
    memset(_newImage, 0, EA_IMG_BUF_SZ);
}

// update screen  _newImage -> _oldImage
void EaEpaper::write_disp(void)
{
    epd_.begin();
    epd_.setFactor(readTemperature()/100);
    epd_.image(_oldImage, _newImage);
    epd_.end();

    memcpy(_oldImage, _newImage, EA_IMG_BUF_SZ);
}

// read LM75A sensor on board to calculate display speed
int32_t EaEpaper::readTemperature()
{
    char buf[2];
    int32_t t = 0;
    char reg = LM75A_CMD_TEMP;

    i2c_.write(LM75A_ADDRESS, &reg, 1);
    i2c_.read(LM75A_ADDRESS, buf, 2);

    t = ((buf[0] << 8) | (buf[1]));

    return ((t * 100) >> 8);
}

int32_t EaEpaper::getCharacterSize(int c)
{
    if ((c < 32) || (c > 127)) return -1;   // test char range
    switch(c)
    {
    	case 32:
		case 33:
		case 34:
		case 39:
		case 40:
		case 41:
		case 42:
		case 44:
		case 45:
		case 46:
		case 47:
		case 58:
		case 59:
		case 60:
		case 62:
		case 91:
		case 92:
		case 93:
		case 94:
		case 96:
		case 99:
		case 106:
		case 123:
		case 124:
		case 127:
			return 22;
		default:
			return 44;
    }
}

int32_t EaEpaper::getFontIndex(int c)
{
	int32_t sum = 0;
	int i = 32;
    if ((c < 32) || (c > 127)) return -1;   // test char range
	while(i<c){
		sum += getCharacterSize(i++);
	};
	return sum;
}
// set one pixel in buffer _newImage

void EaEpaper::pixel(int x, int y, int color)
{
    // first check parameter
    if(x > 263 || y > 175 || x < 0 || y < 0) return;

    if(draw_mode == NORMAL) {
        if(color == 0)
            _newImage[(x / 8) + ((y-1) * 33)] &= ~(1 << (x%8));  // erase pixel
        else
            _newImage[(x / 8) + ((y-1) * 33)] |= (1 << (x%8));   // set pixel
    } else { // XOR mode
        if(color == 1)
            _newImage[(x / 8) + ((y-1) * 33)] ^= (1 << (x%8));   // xor pixel
    }
}

// clear screen
void EaEpaper::cls(void)
{
    memset(_newImage, 0, EA_IMG_BUF_SZ);  // clear display buffer
}

// print line
void EaEpaper::line(int x0, int y0, int x1, int y1, int color)
{
    int   dx = 0, dy = 0;
    int   dx_sym = 0, dy_sym = 0;
    int   dx_x2 = 0, dy_x2 = 0;
    int   di = 0;

    dx = x1-x0;
    dy = y1-y0;

    if (dx > 0) {
        dx_sym = 1;
    } else {
        dx_sym = -1;
    }

    if (dy > 0) {
        dy_sym = 1;
    } else {
        dy_sym = -1;
    }

    dx = dx_sym*dx;
    dy = dy_sym*dy;

    dx_x2 = dx*2;
    dy_x2 = dy*2;

    if (dx >= dy) {
        di = dy_x2 - dx;
        while (x0 != x1) {

            pixel(x0, y0, color);
            x0 += dx_sym;
            if (di<0) {
                di += dy_x2;
            } else {
                di += dy_x2 - dx_x2;
                y0 += dy_sym;
            }
        }
        pixel(x0, y0, color);
    } else {
        di = dx_x2 - dy;
        while (y0 != y1) {
            pixel(x0, y0, color);
            y0 += dy_sym;
            if (di < 0) {
                di += dx_x2;
            } else {
                di += dx_x2 - dy_x2;
                x0 += dx_sym;
            }
        }
        pixel(x0, y0, color);
    }
}

// print rect
void EaEpaper::rect(int x0, int y0, int x1, int y1, int color)
{

    if (x1 > x0) line(x0,y0,x1,y0,color);
    else  line(x1,y0,x0,y0,color);

    if (y1 > y0) line(x0,y0,x0,y1,color);
    else line(x0,y1,x0,y0,color);

    if (x1 > x0) line(x0,y1,x1,y1,color);
    else  line(x1,y1,x0,y1,color);

    if (y1 > y0) line(x1,y0,x1,y1,color);
    else line(x1,y1,x1,y0,color);
}

// print filled rect
void EaEpaper::fillrect(int x0, int y0, int x1, int y1, int color)
{
    int l,c,i;
    if(x0 > x1) {
        i = x0;
        x0 = x1;
        x1 = i;
    }

    if(y0 > y1) {
        i = y0;
        y0 = y1;
        y1 = i;
    }

    for(l = x0; l<= x1; l ++) {
        for(c = y0; c<= y1; c++) {
            pixel(l,c,color);
        }
    }
}

// print circle
void EaEpaper::circle(int x0, int y0, int r, int color)
{
    int x = -r, y = 0, err = 2-2*r, e2;
    do {
        pixel(x0-x, y0+y,color);
        pixel(x0+x, y0+y,color);
        pixel(x0+x, y0-y,color);
        pixel(x0-x, y0-y,color);
        e2 = err;
        if (e2 <= y) {
            err += ++y*2+1;
            if (-x == y && e2 <= x) e2 = 0;
        }
        if (e2 > x) err += ++x*2+1;
    } while (x <= 0);

}

// print filled circle
void EaEpaper::fillcircle(int x0, int y0, int r, int color)
{
    int x = -r, y = 0, err = 2-2*r, e2;
    do {
        line(x0-x, y0-y, x0-x, y0+y, color);
        line(x0+x, y0-y, x0+x, y0+y, color);
        e2 = err;
        if (e2 <= y) {
            err += ++y*2+1;
            if (-x == y && e2 <= x) e2 = 0;
        }
        if (e2 > x) err += ++x*2+1;
    } while (x <= 0);
}

// set drawing mode
void EaEpaper::setmode(int mode)
{
    draw_mode = mode;
}

// set cursor position
void EaEpaper::locate(int x, int y)
{
    char_x = x;
    char_y = y;
}

// calc char columns
int EaEpaper::columns()
{
    return width() / font[1];
}

// calc char rows
int EaEpaper::rows()
{
    return height() / font[2];
}

// print char
int EaEpaper::_putc(int value)
{
    if (value == '\n') {    // new line
        char_x = 0;
        char_y = char_y + font[2];
        if (char_y >= height() - font[2]) {
            char_y = 0;
        }
    } else {
        character(char_x, char_y, value);
    }
    return value;
}

// paint char out of font
void EaEpaper::character(int x, int y, int c)
{
    unsigned int hor,vert,offset,bpl,j,i,b;
    unsigned char* zeichen;
    unsigned char z,w;

    if ((c < 31) || (c > 127)) return;   // test char range

    // read font parameter from start of array
    offset = font[0];                    // bytes / char
    hor = font[1];                       // get hor size of font
    vert = font[2];                      // get vert size of font
    bpl = font[3];                       // bytes per line

    if (char_x + hor > width()) {
        char_x = 0;
        char_y = char_y + vert;
        if (char_y >= height() - font[2]) {
            char_y = 0;
        }
    }

    zeichen = &font[((c -32) * offset) + 4]; // start of char bitmap
    w = zeichen[0];                          // width of actual char
    // construct the char into the buffer
    for (j=0; j<vert; j++) {  //  vert line
        for (i=0; i<hor; i++) {   //  horz line
            z =  zeichen[bpl * i + ((j & 0xF8) >> 3)+1];
            b = 1 << (j & 0x07);
            if (( z & b ) == 0x00) {
                pixel(x+i,y+j,0);
            } else {
                pixel(x+i,y+j,1);
            }

        }
    }

    char_x += w;
}

/**************************************
* @func draw character in (x,y)
* @param x [0-263] x location of the left top corner of the character
* @param y [0-175] y location of the left top corner of the character
* @ret -1 when invalid character
	    -2 when parameter out of range
	    0   when success
**************************************/
int EaEpaper::putchar(int x, int y, char c)
{
	int32_t size = getCharacterSize(c);
	char *pfont = NULL;
	unsigned short i = 0, j = 0;
			
	if(size < 0)
		return -1;	//invalid character
	else if(size == 22)
	{
		if(y - 8 < 0 || y >= height() || x >= width() || x + 22 > width())
			return -3; //y location out of range
		pfont = (char *)&font[getFontIndex(c)];
		for(i = x; i<x+22 ; i++) //x location of current pixel
			for(j = y; j>y-8 ; j--)
			{
				if(pfont[i-x] & (0x01 << (y-j)))
					pixel(i,j,1);
				else
					pixel(i,j,0);
			}
	}
	else if(size == 44)
	{
		if(y - 16 < 0 || y >= height() || x >= width() || x + 22 > width())
			return -3;
		pfont = (char *)&font[getFontIndex(c)];
		for(i = x; i<x+22 ; i++) //x location of current pixel
			for(j = y; j>y-16 ; j--)
			{
				if(pfont[2*(i-x)+(j>y-8?0:1)] & (0x01 << (y-j)%8))
					pixel(i,j,1);
				else
					pixel(i,j,0);
			}
	}
	return 0;
}

// set actual font
void EaEpaper::set_font(unsigned char* f)
{
    font = f;
}

void EaEpaper::set_rofont(const unsigned char * f)
{
	rofont = f;
}
void EaEpaper::print_bm(Bitmap bm, int x, int y)
{
    int h,v,b;
    char d;

    for(v=0; v < bm.ySize; v++) {   // lines
        for(h=0; h < bm.xSize; h++) { // pixel
            if(h + x > width()) break;
            if(v + y > height()) break;
            d = bm.data[bm.Byte_in_Line * v + ((h & 0xF8) >> 3)];
            //b = 0x80 >> (h & 0x07);
						b = 0x01 << (h & 0x07);
            if((d & b) == 0) {
                pixel(x+h,y+v,0);
            } else {
                pixel(x+h,y+v,1);
            }
        }
    }

}

void EaEpaper::print_inversebm(Bitmap bm,int x,int y)
{
	int h,v,b;
	char d;

	for(v=0; v < bm.ySize; v++) {	// lines
		for(h=0; h < bm.xSize; h++) { // pixel
			if(h + x > width()) break;
			if(v + y > height()) break;
			d = bm.data[bm.Byte_in_Line * v + ((h & 0xF8) >> 3)];
			//b = 0x80 >> (h & 0x07);
						b = 0x01 << (h & 0x07);
			if((d & b) == 0) {
				pixel(x+h,y+v,1);
			} else {
				pixel(x+h,y+v,0);
			}
		}
	}

}

/***************************************
* @func print string
* @param
* @return 0  when success
		  -1 when null string
		  -2 when invalid character among string
		  -3 when string out of display range
***************************************/
int EaEpaper::print_string(char * str, int x, int y)
{
	unsigned short i = 0;
	int ret = 0;
	
	if(!str)
		return -1;
		
	while(str[i] != '\0')
	{
		ret = putchar(x,y,str[i++]);
		if(ret == -1)
			return -2; //invalid character
		else if(ret == -2)
			return -3; //param out of range 
		y -= cwidth(str[i]);
	}
	return 0;
}	

