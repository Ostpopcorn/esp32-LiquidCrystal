#ifndef LiquidCrystal_h
#define LiquidCrystal_h



#include <stdio.h>
#include <inttypes.h>
// #include "Print.h"

// commands
#define LCD_CLEARDISPLAY 0x01
#define LCD_RETURNHOME 0x02
#define LCD_ENTRYMODESET 0x04
#define LCD_DISPLAYCONTROL 0x08
#define LCD_CURSORSHIFT 0x10
#define LCD_FUNCTIONSET 0x20
#define LCD_SETCGRAMADDR 0x40
#define LCD_SETDDRAMADDR 0x80

// flags for display entry mode
#define LCD_ENTRYRIGHT 0x00
#define LCD_ENTRYLEFT 0x02
#define LCD_ENTRYSHIFTINCREMENT 0x01
#define LCD_ENTRYSHIFTDECREMENT 0x00

// flags for display on/off control
#define LCD_DISPLAYON 0x04
#define LCD_DISPLAYOFF 0x00
#define LCD_CURSORON 0x02
#define LCD_CURSOROFF 0x00
#define LCD_BLINKON 0x01
#define LCD_BLINKOFF 0x00

// flags for display/cursor shift
#define LCD_DISPLAYMOVE 0x08
#define LCD_CURSORMOVE 0x00
#define LCD_MOVERIGHT 0x04
#define LCD_MOVELEFT 0x00

// flags for function set
#define LCD_8BITMODE 0x10
#define LCD_4BITMODE 0x00
#define LCD_2LINE 0x08
#define LCD_1LINE 0x00
#define LCD_5x10DOTS 0x04
#define LCD_5x8DOTS 0x00

class LiquidCrystal {
public:
  typedef enum bit_mode{
    EIGHT_BIT= 1,
    FOUR_BIT,
  } bit_mode;
  typedef enum ctrl_pins{
    NO_PIN = 0,
    RS_PIN,
    RW_PIN,
    E_PIN,
  } ctrl_pins;

  LiquidCrystal();
  LiquidCrystal(bit_mode mode);
  virtual ~LiquidCrystal();
  void init();
  
  void begin(uint8_t cols, uint8_t rows, uint8_t charsize = LCD_5x8DOTS);

  void clear();
  void home();

  void noDisplay();
  void display();
  void noBlink();
  void blink();
  void noCursor();
  void cursor();
  void scrollDisplayLeft();
  void scrollDisplayRight();
  void leftToRight();
  void rightToLeft();
  void autoscroll();
  void noAutoscroll();

  void setRowOffsets(int row1, int row2, int row3, int row4);
  void createChar(uint8_t, uint8_t[]);
  void setCursor(uint8_t, uint8_t); 
  virtual size_t write(uint8_t);
  void command(uint8_t);
  
  // Transport
  
  void send(uint8_t value, uint8_t mode, bool only_send_four_bits = false);

  bit_mode get_bit_mode();
  
protected:
  void writeDataBuffer(uint8_t data_in);

  void writeCtrlBuffer(ctrl_pins pin, uint8_t value);
  uint8_t get_data_buffer();
  uint8_t get_ctrl_buffer();
  uint8_t get_ctrl_buffer_mask(ctrl_pins pin);

  virtual void latch(bool only_send_four_bits = false);
  virtual void pulseEnable();
  
private:

  uint8_t _displayfunction;
  uint8_t _displaycontrol;
  uint8_t _displaymode;

  uint8_t _initialized;

  uint8_t _numlines;
  uint8_t _row_offsets[4];

  uint8_t _data_buffer = 0;
  uint8_t _ctrl_buffer = 0; // RS(4) RW(2) E(1)
  bit_mode mode;
};

#endif
