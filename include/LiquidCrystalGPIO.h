#ifndef LiquidCrystalGPIO_h
#define LiquidCrystalGPIO_h
#include "LiquidCrystal.h"

#include <inttypes.h>
#include "driver/gpio.h"

class LiquidCrystalGPIO : public LiquidCrystal {
public:
    LiquidCrystalGPIO(bit_mode pinmode,gpio_num_t rs, gpio_num_t rw, gpio_num_t enable,
			 gpio_num_t d0, gpio_num_t d1, gpio_num_t d2, gpio_num_t d3,
			 gpio_num_t d4, gpio_num_t d5, gpio_num_t d6, gpio_num_t d7);

protected:
    virtual void latch(bool only_send_four_bits = false) override;
    void write4bits(uint8_t value);
    void write8bits(uint8_t value);
    virtual void pulseEnable(void) override;
    gpio_num_t _rs_pin; // LOW: command.  HIGH: character.
    gpio_num_t _rw_pin; // LOW: write to LCD.  HIGH: read from LCD.
    gpio_num_t _enable_pin; // activated by a HIGH pulse.
    gpio_num_t _data_pins[8] = {GPIO_NUM_NC,GPIO_NUM_NC,GPIO_NUM_NC,GPIO_NUM_NC,
                                GPIO_NUM_NC,GPIO_NUM_NC,GPIO_NUM_NC,GPIO_NUM_NC};
           
private:
    void setDataPin(uint8_t pin, gpio_num_t gpio);
    void setCtrlPin(ctrl_pins pin, gpio_num_t gpio);

};


#endif