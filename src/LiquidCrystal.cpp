#include "LiquidCrystal.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include <stdio.h>
#include <string.h>
#include <inttypes.h>
#include "esp_log.h"

#include "LcdTransport.h"
#include "delayMicroseconds.h"

#define TAG "LCD"
// DEssa är bara så att den tillfälligt kompilerar.
#define HIGH 1
#define LOW 0
// #define digitalWrite(pin,mode) ESP_LOGI(TAG,"digitalWrite call... fix pliz")
// #define delayMicroseconds(time) ESP_LOGI(TAG,"delayMicroseconds call... fix pliz")

// When the display powers up, it is configured as follows:
//
// 1. Display clear
// 2. Function set: 
//    DL = 1; 8-bit interface data 
//    N = 0; 1-line display 
//    F = 0; 5x8 dot character font 
// 3. Display on/off control: 
//    D = 0; Display off 
//    C = 0; Cursor off 
//    B = 0; Blinking off 
// 4. Entry mode set: 
//    I/D = 1; Increment by 1 
//    S = 0; No shift 
//
// Note, however, that resetting the Arduino doesn't reset the LCD, so we
// can't assume that its in that state when a sketch starts (and the
// LiquidCrystal constructor is called).
/*
LiquidCrystal::LiquidCrystal(gpio_num_t rs, gpio_num_t enable,
			     gpio_num_t d0, gpio_num_t d1, gpio_num_t d2, gpio_num_t d3,
			     gpio_num_t d4, gpio_num_t d5, gpio_num_t d6, gpio_num_t d7)
{
  init(0, rs, GPIO_NUM_NC, enable, d0, d1, d2, d3, d4, d5, d6, d7);
}

LiquidCrystal::LiquidCrystal(gpio_num_t rs, gpio_num_t rw, gpio_num_t enable,
			     gpio_num_t d0, gpio_num_t d1, gpio_num_t d2, gpio_num_t d3)
{
  init(1, rs, rw, enable, d0, d1, d2, d3, GPIO_NUM_NC, GPIO_NUM_NC, GPIO_NUM_NC, GPIO_NUM_NC);
}

LiquidCrystal::LiquidCrystal(gpio_num_t rs,  gpio_num_t enable,
			     gpio_num_t d0, gpio_num_t d1, gpio_num_t d2, gpio_num_t d3)
{
  init(1, rs, GPIO_NUM_NC, enable, d0, d1, d2, d3, GPIO_NUM_NC, GPIO_NUM_NC, GPIO_NUM_NC, GPIO_NUM_NC);
}

*/

template<> template<>
inline void LiquidCrystal<LcdTransport>::command(uint8_t value);
template<>
inline size_t LiquidCrystal<LcdTransport>::write(uint8_t value);
template<>
void LiquidCrystal<LcdTransport>::begin(uint8_t cols, uint8_t lines, uint8_t dotsize);


template <>
LiquidCrystal<LcdTransport>::LiquidCrystal(LcdTransport transport){
  transport = transport;
  if (transport.get_bit_mode() == LcdTransport::bit_mode::FOUR_BIT)
    _displayfunction = LCD_4BITMODE | LCD_1LINE | LCD_5x8DOTS;
  else 
    _displayfunction = LCD_8BITMODE | LCD_1LINE | LCD_5x8DOTS;
  begin(16, 1);  
}

template<class transport_t>
LiquidCrystal<transport_t>::LiquidCrystal(){

}

template<class transport_t>
LiquidCrystal<transport_t>::~LiquidCrystal(){

}

template<>
void LiquidCrystal<LcdTransport>::begin(uint8_t cols, uint8_t lines, uint8_t dotsize) {
  
  if (lines > 1) {
    _displayfunction |= LCD_2LINE;
  }
  _numlines = lines;

  setRowOffsets(0x00, 0x40, 0x00 + cols, 0x40 + cols);  

  // for some 1 line displays you can select a 10 pixel high font
  if ((dotsize != LCD_5x8DOTS) && (lines == 1)) {
    _displayfunction |= LCD_5x10DOTS;
  }

  // SEE PAGE 45/46 FOR INITIALIZATION SPECIFICATION!
  // according to datasheet, we need at least 40ms after power rises above 2.7V
  // before sending commands. Arduino can turn on way before 4.5V so we'll wait 50
  
  vTaskDelay(50 / portTICK_PERIOD_MS);
  

  // Now we pull both RS and R/W low to begin commands
  
  //gpio_set_level(_rs_pin, LOW);
  //gpio_set_level(_enable_pin, LOW);
  //if (_rw_pin != GPIO_NUM_NC) { 
  //  gpio_set_level(_rw_pin, LOW);
  //}
  
  transport.send(0,0);
  //put the LCD into 4 bit or 8 bit mode
  if (! (_displayfunction & LCD_8BITMODE)) {
    // this is according to the hitachi HD44780 datasheet
    // figure 24, pg 46

    // we start in 8bit mode, try to set 4 bit mode
    // write4bits(0x03); Kanske blir något problem här, håll ögat öppet!
    command(0x03);
    vTaskDelay(5 / portTICK_PERIOD_MS);// wait min 4.1ms

    // second try
    command(0x03);
    vTaskDelay(5 / portTICK_PERIOD_MS); // wait min 4.1ms
    
    // third go!
    command(0x03); 
    delayMicroseconds(150);

    // finally, set to 4-bit interface
    command(0x02); 
  } else {
    // this is according to the hitachi HD44780 datasheet
    // page 45 figure 23

    // Send function set command sequence
    command(LCD_FUNCTIONSET | _displayfunction);
    vTaskDelay(5 / portTICK_PERIOD_MS);  // wait more than 4.1ms

    // second try
    command(LCD_FUNCTIONSET | _displayfunction);
    delayMicroseconds(150);

    // third go
    command(LCD_FUNCTIONSET | _displayfunction);
  }

  // finally, set # lines, font size, etc.
  command(LCD_FUNCTIONSET | _displayfunction);  

  // turn the display on with no cursor or blinking default
  _displaycontrol = LCD_DISPLAYON | LCD_CURSOROFF | LCD_BLINKOFF;  
  display();

  // clear it off
  clear();

  // Initialize to default text direction (for romance languages)
  _displaymode = LCD_ENTRYLEFT | LCD_ENTRYSHIFTDECREMENT;
  // set the entry mode
  command(LCD_ENTRYMODESET | _displaymode);

}

template<class transport_t>
void LiquidCrystal<transport_t>::begin(uint8_t cols, uint8_t lines, uint8_t dotsize) {
  
}

template<class transport_t>
void LiquidCrystal<transport_t>::init()
{
  
}

template<class transport_t>
void LiquidCrystal<transport_t>::setRowOffsets(int row0, int row1, int row2, int row3)
{
  _row_offsets[0] = row0;
  _row_offsets[1] = row1;
  _row_offsets[2] = row2;
  _row_offsets[3] = row3;
}

/********** high level commands, for the user! */
template<class transport_t>
void LiquidCrystal<transport_t>::clear()
{
  command(LCD_CLEARDISPLAY);  // clear display, set cursor position to zero
    vTaskDelay(2 / portTICK_PERIOD_MS); // this command takes a long time!
}

template<class transport_t>
void LiquidCrystal<transport_t>::home()
{
  command(LCD_RETURNHOME);  // set cursor position to zero
  vTaskDelay(2 / portTICK_PERIOD_MS);  // this command takes a long time!
}

template<class transport_t>
void LiquidCrystal<transport_t>::setCursor(uint8_t col, uint8_t row)
{
  const size_t max_lines = sizeof(_row_offsets) / sizeof(*_row_offsets);
  if ( row >= max_lines ) {
    row = max_lines - 1;    // we count rows starting w/0
  }
  if ( row >= _numlines ) {
    row = _numlines - 1;    // we count rows starting w/0
  }
  
  command(LCD_SETDDRAMADDR | (col + _row_offsets[row]));
}

// Turn the display on/off (quickly)
template<class transport_t>
void LiquidCrystal<transport_t>::noDisplay() {
  _displaycontrol &= ~LCD_DISPLAYON;
  command(LCD_DISPLAYCONTROL | _displaycontrol);
}
template<class transport_t>
void LiquidCrystal<transport_t>::display() {
  _displaycontrol |= LCD_DISPLAYON;
  command(LCD_DISPLAYCONTROL | _displaycontrol);
}

// Turns the underline cursor on/off
template<class transport_t>
void LiquidCrystal<transport_t>::noCursor() {
  _displaycontrol &= ~LCD_CURSORON;
  command(LCD_DISPLAYCONTROL | _displaycontrol);
}
template<class transport_t>
void LiquidCrystal<transport_t>::cursor() {
  _displaycontrol |= LCD_CURSORON;
  command(LCD_DISPLAYCONTROL | _displaycontrol);
}

// Turn on and off the blinking cursor
template<class transport_t>
void LiquidCrystal<transport_t>::noBlink() {
  _displaycontrol &= ~LCD_BLINKON;
  command(LCD_DISPLAYCONTROL | _displaycontrol);
}
template<class transport_t>
void LiquidCrystal<transport_t>::blink() {
  _displaycontrol |= LCD_BLINKON;
  command(LCD_DISPLAYCONTROL | _displaycontrol);
}

// These commands scroll the display without changing the RAM
template<class transport_t>
void LiquidCrystal<transport_t>::scrollDisplayLeft(void) {
  command(LCD_CURSORSHIFT | LCD_DISPLAYMOVE | LCD_MOVELEFT);
}
template<class transport_t>
void LiquidCrystal<transport_t>::scrollDisplayRight(void) {
  command(LCD_CURSORSHIFT | LCD_DISPLAYMOVE | LCD_MOVERIGHT);
}

// This is for text that flows Left to Right
template<class transport_t>
void LiquidCrystal<transport_t>::leftToRight(void) {
  _displaymode |= LCD_ENTRYLEFT;
  command(LCD_ENTRYMODESET | _displaymode);
}

// This is for text that flows Right to Left
template<class transport_t>
void LiquidCrystal<transport_t>::rightToLeft(void) {
  _displaymode &= ~LCD_ENTRYLEFT;
  command(LCD_ENTRYMODESET | _displaymode);
}

// This will 'right justify' text from the cursor
template<class transport_t>
void LiquidCrystal<transport_t>::autoscroll(void) {
  _displaymode |= LCD_ENTRYSHIFTINCREMENT;
  command(LCD_ENTRYMODESET | _displaymode);
}

// This will 'left justify' text from the cursor
template<class transport_t>
void LiquidCrystal<transport_t>::noAutoscroll(void) {
  _displaymode &= ~LCD_ENTRYSHIFTINCREMENT;
  command(LCD_ENTRYMODESET | _displaymode);
}

// Allows us to fill the first 8 CGRAM locations
// with custom characters
template<class transport_t>
void LiquidCrystal<transport_t>::createChar(uint8_t location, uint8_t charmap[]) {
  location &= 0x7; // we only have 8 locations 0-7
  command(LCD_SETCGRAMADDR | (location << 3));
  for (int i=0; i<8; i++) {
    write(charmap[i]);
  }
}


template<>
inline void LiquidCrystal<LcdTransport>::command(uint8_t value) {
  transport.send(value, LOW);
}
template<>
inline size_t LiquidCrystal<LcdTransport>::write(uint8_t value) {
  transport.send(value, HIGH);
  return 1; // assume sucess
}