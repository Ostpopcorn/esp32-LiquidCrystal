#ifndef LiquidCrystalTransport_h
#define LiquidCrystalTransport_h

#include <inttypes.h>


class LcdTransport{
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
  
  LcdTransport();
  LcdTransport(bit_mode mode);
  virtual ~LcdTransport();
  void send(uint8_t value, uint8_t mode, bool only_send_four_bits = false);

  bit_mode get_bit_mode();
protected:
  void writeDataBuffer(uint8_t data_in);

  void writeCtrlBuffer(ctrl_pins pin, uint8_t value);
  uint8_t get_data_buffer();
  uint8_t get_ctrl_buffer();
  uint8_t get_ctrl_buffer_mask(ctrl_pins pin);

  virtual void latch(bool only_send_four_bits = false) = 0;
  virtual void pulseEnable() = 0;
private:
  uint8_t _data_buffer = 0;
  uint8_t _ctrl_buffer = 0; // RS(4) RW(2) E(1)
  bit_mode mode;
};




#endif