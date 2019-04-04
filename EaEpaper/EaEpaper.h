/* mbed library for 264*176 pixel 2.7 INCH E-PAPER DISPLAY from Pervasive Displays
 * Copyright (c) 2013 Peter Drescher - DC2PD
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */


// 09.11.2013   initial Version

#ifndef EAEPAPER_H
#define EAEPAPER_H

/**
 * Includes
 */
#include "mbed.h"
#include "EPD.h"
#include "GraphicsDisplay.h"

// we have to double buffer the display  
#define EA_IMG_BUF_SZ (5808) // 264 x 176 / 8 = 5808

/** Draw mode
  * NORMAl
  * XOR set pixel by xor of the screen
  */
enum {NORMAL,XOR};

/** Bitmap
 */
struct Bitmap{
    int xSize;
    int ySize;
    int Byte_in_Line;
    char* data;
    };

/* color definitions   */
#define Black           0x0
#define White           0x1   

/** Display control class, based on GraphicsDisplay and TextDisplay
 *
 * Example with pinning for KL25Z:
 * @code
 * #include "mbed.h"
 * #include "EaEpaper.h"
 * #include "Arial28x28.h" 
 * #include "Arial12x12.h"
 * #include "font_big.h"
 * #include "graphics.h"

 * // the E-Paper board from embedded artists has a LM75 temp sensor to compensate the temperature effect. If it is cold the display reacts slower.
 * // The LM75 need a I2C -> 2 pins : sda and scl 
 * // The display data is written via SPI -> 3 pins : mosi,miso,sck
 * // There are also some control signals 
 * // The pwm pin has to be connected to a PWM enabled pin : pwm
 * // The other signals are connected to normal IO`s
 * //              
 * EaEpaper epaper(PTD7,            // PWR_CTRL
 *                 PTD6,            // BORDER
 *                 PTE31,           // DISCHARGE
 *                 PTA17,           // RESET_DISP
 *                 PTA16,           // BUSY
 *                 PTC17,           // SSEL
 *                 PTD4,            // PWM
 *                 PTD2,PTD3,PTD1,  // MOSI,MISO,SCLK
 *                 PTE0,PTE1);      // SDA,SCL 
 *
 * int main() {
 *
 *   epaper.cls();                                  // clear screen                        
 *   epaper.set_font((unsigned char*) Arial28x28);  // select the font
 *   epaper.locate(5,20);                           // set cursor
 *   epaper.printf("Hello Mbed");                   // print text
 *   epaper.rect(3,15,150,50,1);                    // draw frame
 *   epaper.write_disp();                           // update screen
 *
 * @endcode
 */
 

class EaEpaper : public GraphicsDisplay  {

public:

    /**
     * Constructor.
     */ 
    EaEpaper(PinName p_on, PinName border, PinName discharge, PinName reset, PinName busy, PinName cs,  // IO-Pins
             PinName pwm,                                                                               // PWM Pin        
             PinName mosi, PinName miso, PinName sck,                                                   // SPI
             PinName sda, PinName scl,                                                                  // I2C
             const char* name ="E_Paper");
    
    /** Get the width of the screen in pixel
      *
      * @param
      * @returns width of screen in pixel
      *
      */
    virtual int width();
	int cwidth(int c);

    /** Get the height of the screen in pixel
     *
     * @returns height of screen in pixel
     *
     */
    virtual int height();
	int cheight();
    
    
    /**
     * Clear the display
     */    
    void clear();
    
    /**
     * Write image buffer to display
     */    
    void write_disp(void);
    
    /**
     * set or reset a single pixel
     *
     * @param x horizontal position
     * @param y vertical position
     * @param color : 0 white, 1 black
     */ 
    virtual void pixel(int x, int y, int color);
    
    /** Fill the screen with white
     *
     */ 
    virtual void cls(void);
    
    /** draw a 1 pixel line
     *
     * @param x0,y0 start point
     * @param x1,y1 stop point
     * @param color  : 0 white, 1 black
     */  
    void line(int x0, int y0, int x1, int y1, int color);
    
    /** draw a rect
     *
     * @param x0,y0 top left corner
     * @param x1,y1 down right corner
     * @param color : 0 white, 1 black
     */    
    void rect(int x0, int y0, int x1, int y1, int color);
    
    /** draw a filled rect
     *
     * @param x0,y0 top left corner
     * @param x1,y1 down right corner
     * @param color : 0 white, 1 black
     */  
    void fillrect(int x0, int y0, int x1, int y1, int color);
    
    /** draw a circle
     *
     * @param x0,y0 center
     * @param r radius
     * @param color : 0 white, 1 black                                                          *
     */    
    void circle(int x0, int y0, int r, int color);
    
    /** draw a filled circle
     *
     * @param x0,y0 center
     * @param r radius
     * @param color : 0 white, 1 black                                                                 *
     */  
    void fillcircle(int x0, int y0, int r, int color);
    
    /** set drawing mode
     *  @param NORMAL : paint normal color, XOR : paint XOR of current pixels
     */
    void setmode(int mode);
    
    /** setup cursor position
     *
     * @param x x-position (top left)
     * @param y y-position 
     */   
    virtual void locate(int x, int y);
    
    /** calculate the max number of char in a line
     *
     * @returns max columns
     * depends on actual font size
     */ 
    virtual int columns();
    
    /** calculate the max number of columns
     *
     * @returns max column
     * depends on actual font size
     */   
    virtual int rows();
    
    /** put a char on the screen
     *
     * @param value char to print
     * @returns printed char
     */
    virtual int _putc(int value);
    
    /** paint a character on given position out of the active font to the screen buffer
     *
     * @param x x-position (top left)
     * @param y y-position 
     * @param c char code
     */
    virtual void character(int x, int y, int c);
	int putchar(int x,int y,char c);
    
    /** select the font to use
     *
     * @param f pointer to font array 
     *                                                                              
     *   font array can created with GLCD Font Creator from http://www.mikroe.com
     *   you have to add 4 parameter at the beginning of the font array to use: 
     *   - the number of byte / char
     *   - the vertial size in pixel
     *   - the horizontal size in pixel
     *   - the number of byte per vertical line
     *   you also have to change the array to char[]
     */  
    void set_font(unsigned char* f);
	void set_rofont(const unsigned char *f);
    
    /** print bitmap to buffer
      *
      * @param bm struct Bitmap in flash
      * @param x  x start
      * @param y  y start 
      *
      */
    void print_bm(Bitmap bm, int x, int y);
	void print_inversebm(Bitmap bm, int x, int y);
	int print_string(char *str,int x,int y);
    
    
    unsigned char* font;
	const unsigned char *rofont;
    unsigned int draw_mode;
    
 private:
    
    EPD_Class epd_;
    I2C i2c_;
    unsigned int char_x;
    unsigned int char_y;
    int32_t readTemperature();
	int32_t getCharacterSize(int c);
	int32_t getFontIndex(int c);
    
};
 
 #endif /* EAEPAPER_H */

