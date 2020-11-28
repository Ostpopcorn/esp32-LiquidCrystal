#include "LcdTransport.h"
#include "freertos/FreeRTOS.h"

#include "esp_log.h"

#define TAG "LCDTRANS"

LcdTransport::LcdTransport():mode{EIGHT_BIT}{

}
LcdTransport::LcdTransport(bit_mode mode):mode{mode}{

}
LcdTransport::~LcdTransport(){
    
}
//-------------------
void LcdTransport::send(uint8_t value, uint8_t mode, bool only_send_four_bits){
    ESP_LOGI(TAG,"send %x",value);
    writeCtrlBuffer(RS_PIN,mode); //gpio_set_level(_rs_pin, mode);

    // if there is a RW pin indicated, set it low to Write, subclasses will deal the rest
    writeCtrlBuffer(RW_PIN,0); //gpio_set_level(_rs_pin, mode);
    writeDataBuffer(value);
    latch(only_send_four_bits);
    pulseEnable();
}

LcdTransport::bit_mode LcdTransport::get_bit_mode(){
    return mode;
}
void LcdTransport::writeDataBuffer(uint8_t data_in){
    _data_buffer = data_in;
}
void LcdTransport::writeCtrlBuffer(ctrl_pins pin, uint8_t value){
    if (pin != NO_PIN){
        // set the position to zero
        _ctrl_buffer &= ~(get_ctrl_buffer_mask(pin));
        // write to that position
        _ctrl_buffer |= ((value==0?0:1) & get_ctrl_buffer_mask(pin));     

    }
}
uint8_t LcdTransport::get_data_buffer(){
    return _data_buffer;
}
uint8_t LcdTransport::get_ctrl_buffer(){
    return _ctrl_buffer;
} 
uint8_t LcdTransport::get_ctrl_buffer_mask(ctrl_pins pin){
    return 1<<(pin-1);
}
