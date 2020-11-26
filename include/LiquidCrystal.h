#ifndef LiquidCrystal_h
#define LiquidCrystal_h

#include "driver/gpio.h"

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


// DENNA SKA BORT SEN--------------------------
#define NOP() asm volatile ("nop")

unsigned long IRAM_ATTR micros()
{
    return (unsigned long) (esp_timer_get_time());
}
void IRAM_ATTR delayMicroseconds(uint32_t us)
{
    uint32_t m = micros();
    if(us){
        uint32_t e = (m + us);
        if(m > e){ //overflow
            while(micros() > e){
                NOP();
            }
        }
        while(micros() < e){
            NOP();
        }
    }
}
//-------------------

class LcdTransport{
public:
  typedef enum bit_mode{
    EIGHT_BIT= 1,
    FOUR_BIT,
  } bit_mode;
  LcdTransport(bit_mode mode){
    this->mode = mode;
  }

  typedef enum ctrl_pins{
    NO_PIN = 0,
    RS_PIN,
    RW_PIN,
    E_PIN,
  } ctrl_pins;
  virtual void init() = 0;
  void send(uint8_t value, uint8_t mode){
    writeCtrlBuffer(RS_PIN,mode); //gpio_set_level(_rs_pin, mode);

    // if there is a RW pin indicated, set it low to Write, subclasses will deal the rest
    writeCtrlBuffer(RW_PIN,0); //gpio_set_level(_rs_pin, mode);
    writeDataBuffer(value);
    latch();
    pulseEnable();
  }

protected:
  void writeDataBuffer(uint8_t data_in){
    _data_buffer = data_in;
  }
  void writeCtrlBuffer(ctrl_pins pin, uint8_t value){
    if (pin != NO_PIN){
      // set the position to zero
      _ctrl_buffer &= ~(1<<(pin-1));
      // write to that position
      _ctrl_buffer |= (value==0?0:1)<<(pin-1);
    }
  }
  uint8_t get_data_buffer(){
    return _data_buffer;
  }
  uint8_t get_ctrl_buffer(){
    return _ctrl_buffer;
  } 
  uint8_t get_ctrl_buffer_mask(ctrl_pins pin){
    return 1<<(ctrl_pins::RS_PIN-1);
  }
  bit_mode get_bit_mode(){
    return mode;
  }

  virtual void latch() = 0;
  virtual void pulseEnable() = 0;
private:
  uint8_t _data_buffer;
  uint8_t _ctrl_buffer; // RS(4) RW(2) E(1)
  bit_mode mode;
};



class LcdTransportGPIO : public LcdTransport{
public:
  LcdTransportGPIO(bit_mode pinmode,gpio_num_t rs, gpio_num_t rw, gpio_num_t enable,
			 gpio_num_t d0, gpio_num_t d1, gpio_num_t d2, gpio_num_t d3,
			 gpio_num_t d4, gpio_num_t d5, gpio_num_t d6, gpio_num_t d7) : LcdTransport(pinmode)
  {
    // Kanske ska kontrollera att den får tillräckligt många pins
    setCtrlPin(ctrl_pins::RS_PIN, rs);
    setCtrlPin(ctrl_pins::RW_PIN, rw);
    setCtrlPin(ctrl_pins::E_PIN, enable);
    setDataPin(0,d0);
    setDataPin(1,d1);
    setDataPin(2,d2);
    setDataPin(3,d3);
    setDataPin(4,d4);
    setDataPin(5,d5);
    setDataPin(6,d6);
    setDataPin(7,d7);
    gpio_config_t io_conf = {
      .pin_bit_mask = 0,
      .mode = GPIO_MODE_OUTPUT,
      .pull_up_en = GPIO_PULLUP_DISABLE,
      .pull_down_en = GPIO_PULLDOWN_DISABLE,
      .intr_type = GPIO_INTR_DISABLE,
    };
    uint64_t pin_mask_ctrl = (1ULL<<_rs_pin) | (1ULL<<_enable_pin);
    if (_rw_pin != GPIO_NUM_NC) { 
      pin_mask_ctrl |= (1ULL<<_rw_pin);
    }
    for (int i=0; i<(get_bit_mode() == EIGHT_BIT ? 8 : 4); ++i)
    {
      pin_mask_ctrl |= (1ULL<<_data_pins[i]);
    } 
    io_conf.pin_bit_mask = pin_mask_ctrl;
    gpio_config(&io_conf);
  }
protected:
  void latch() override {
    // alla nya data ska ut på samtliga linor, ska optimeras sen så att bara de som
    // ändrats på ska uppdateras
    if (get_bit_mode() == EIGHT_BIT) {
      write8bits(get_data_buffer()); 
    } else {
      write4bits(get_data_buffer()>>4);
      write4bits(get_data_buffer());
    }
    gpio_set_level(_rs_pin, (get_ctrl_buffer() & get_ctrl_buffer_mask(ctrl_pins::RS_PIN)));
    gpio_set_level(_rw_pin, (get_ctrl_buffer() & get_ctrl_buffer_mask(ctrl_pins::RW_PIN)));
    gpio_set_level(_enable_pin, (get_ctrl_buffer() & get_ctrl_buffer_mask(ctrl_pins::E_PIN)));
  }
  void write4bits(uint8_t value) {
    for (int i = 0; i < 4; i++) {
      gpio_set_level(_data_pins[i], (value >> i) & 0x01);
    }

    pulseEnable();
  }
  void write8bits(uint8_t value) {
    for (int i = 0; i < 8; i++) {
      gpio_set_level(_data_pins[i], (value >> i) & 0x01);
    }
    
    pulseEnable();
  }
  void pulseEnable(void) override {
    // Ska se om denna kan köras via latch och de andra funktioner så att koder blir snyggare
    gpio_set_level(_enable_pin, 0);
    delayMicroseconds(1);    
    gpio_set_level(_enable_pin, 1);
    delayMicroseconds(1);    // enable pulse must be >450ns
    gpio_set_level(_enable_pin, 0);
    delayMicroseconds(100);   // commands need > 37us to settle
  }
  gpio_num_t _rs_pin; // LOW: command.  HIGH: character.
  gpio_num_t _rw_pin; // LOW: write to LCD.  HIGH: read from LCD.
  gpio_num_t _enable_pin; // activated by a HIGH pulse.
  gpio_num_t _data_pins[8] = {GPIO_NUM_NC,GPIO_NUM_NC,GPIO_NUM_NC,GPIO_NUM_NC,
                              GPIO_NUM_NC,GPIO_NUM_NC,GPIO_NUM_NC,GPIO_NUM_NC};

private:

  void setDataPin(uint8_t pin, gpio_num_t gpio){
    _data_pins[pin] = gpio;
  }
  void setCtrlPin(ctrl_pins pin, gpio_num_t gpio){
    if (pin == ctrl_pins::RS_PIN){
      _rs_pin = gpio;
    }
    else if (pin == ctrl_pins::RW_PIN){
      _rw_pin = gpio;
    }
    else if (pin == ctrl_pins::E_PIN){
      _enable_pin = gpio;
    }
  }
};


class LcdTransport74HC595{
protected:
  gpio_num_t _data; // LOW: command.  HIGH: character.
  gpio_num_t _clk; // LOW: write to LCD.  HIGH: read from LCD.
  gpio_num_t _latch; // activated by a HIGH pulse.

};
class LiquidCrystal {
public:
  LiquidCrystal(gpio_num_t rs, gpio_num_t enable,
		gpio_num_t d0, gpio_num_t d1, gpio_num_t d2, gpio_num_t d3,
		gpio_num_t d4, gpio_num_t d5, gpio_num_t d6, gpio_num_t d7);
  LiquidCrystal(gpio_num_t rs, gpio_num_t rw, gpio_num_t enable,
		gpio_num_t d0, gpio_num_t d1, gpio_num_t d2, gpio_num_t d3,
		gpio_num_t d4, gpio_num_t d5, gpio_num_t d6, gpio_num_t d7);
  LiquidCrystal(gpio_num_t rs, gpio_num_t rw, gpio_num_t enable,
		gpio_num_t d0, gpio_num_t d1, gpio_num_t d2, gpio_num_t d3);
  LiquidCrystal(gpio_num_t rs, gpio_num_t enable,
		gpio_num_t d0, gpio_num_t d1, gpio_num_t d2, gpio_num_t d3);

  void init(uint8_t fourbitmode, gpio_num_t rs, gpio_num_t rw, gpio_num_t enable,
	    gpio_num_t d0, gpio_num_t d1, gpio_num_t d2, gpio_num_t d3,
	    gpio_num_t d4, gpio_num_t d5, gpio_num_t d6, gpio_num_t d7);
    
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
  
private:
  void send(uint8_t, uint8_t);
  void write4bits(uint8_t);
  void write8bits(uint8_t);
  void pulseEnable();

  gpio_num_t _rs_pin; // LOW: command.  HIGH: character.
  gpio_num_t _rw_pin; // LOW: write to LCD.  HIGH: read from LCD.
  gpio_num_t _enable_pin; // activated by a HIGH pulse.
  gpio_num_t _data_pins[8];

  uint8_t _displayfunction;
  uint8_t _displaycontrol;
  uint8_t _displaymode;

  uint8_t _initialized;

  uint8_t _numlines;
  uint8_t _row_offsets[4];
};

#endif
