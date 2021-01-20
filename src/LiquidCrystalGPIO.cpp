
#include "LiquidCrystalGPIO.h"
#include "delayMicroseconds.h"
#include "esp_log.h"
#define TAG "LCD-GPIO"

LiquidCrystalGPIO::LiquidCrystalGPIO(bit_mode pinmode,gpio_num_t rs, gpio_num_t rw, gpio_num_t enable,
      gpio_num_t d0, gpio_num_t d1, gpio_num_t d2, gpio_num_t d3,
      gpio_num_t d4, gpio_num_t d5, gpio_num_t d6, gpio_num_t d7) : LiquidCrystal(pinmode)
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
  writeCtrlBuffer(LiquidCrystalGPIO::ctrl_pins::E_PIN,0);
  writeCtrlBuffer(LiquidCrystalGPIO::ctrl_pins::RS_PIN,0);
  writeCtrlBuffer(LiquidCrystalGPIO::ctrl_pins::RW_PIN,0);
  writeDataBuffer(0);
  latch();
}
void LiquidCrystalGPIO::latch(bool only_send_four_bits) {
  // alla nya data ska ut på samtliga linor, ska optimeras sen så att bara de som
  // ändrats på ska uppdateras
  if (_rw_pin != GPIO_NUM_NC) { 
    gpio_set_level(_rw_pin, (get_ctrl_buffer() & get_ctrl_buffer_mask(ctrl_pins::RW_PIN)));
  }
  gpio_set_level(_rs_pin, (get_ctrl_buffer() & get_ctrl_buffer_mask(ctrl_pins::RS_PIN)));
  gpio_set_level(_enable_pin, (get_ctrl_buffer() & get_ctrl_buffer_mask(ctrl_pins::E_PIN)));

  if (get_bit_mode() == EIGHT_BIT) {
    write8bits(get_data_buffer()); 
  } else {
    if(!only_send_four_bits){
      write4bits(get_data_buffer()>>4);
    }
    write4bits(get_data_buffer());
  }
  //gpio_set_level(_enable_pin, (get_ctrl_buffer() & get_ctrl_buffer_mask(ctrl_pins::E_PIN)));
}
void LiquidCrystalGPIO::write4bits(uint8_t value) {
  for (int i = 0; i < 4; i++) {
    gpio_set_level(_data_pins[i], (value >> i) & 0x01);
  }

  pulseEnable();
}
void LiquidCrystalGPIO::write8bits(uint8_t value) {
  for (int i = 0; i < 8; i++) {
    gpio_set_level(_data_pins[i], (value >> i) & 0x01);
  }
  
  pulseEnable();
}
void LiquidCrystalGPIO::pulseEnable(void) {
  // Ska se om denna kan köras via latch och de andra funktioner så att koder blir snyggare
  gpio_set_level(_enable_pin, 0);
  delayMicroseconds(2);    
  gpio_set_level(_enable_pin, 1);
  delayMicroseconds(2);    // enable pulse must be >450ns
  gpio_set_level(_enable_pin, 0);
  delayMicroseconds(100);   // commands need > 37us to settle
}
gpio_num_t _rs_pin; // LOW: command.  HIGH: character.
gpio_num_t _rw_pin; // LOW: write to LCD.  HIGH: read from LCD.
gpio_num_t _enable_pin; // activated by a HIGH pulse.
gpio_num_t _data_pins[8] = {GPIO_NUM_NC,GPIO_NUM_NC,GPIO_NUM_NC,GPIO_NUM_NC,
                            GPIO_NUM_NC,GPIO_NUM_NC,GPIO_NUM_NC,GPIO_NUM_NC};


void LiquidCrystalGPIO::setDataPin(uint8_t pin, gpio_num_t gpio){
  _data_pins[pin] = gpio;
}
void LiquidCrystalGPIO::setCtrlPin(ctrl_pins pin, gpio_num_t gpio){
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
