/*
  Copyright (c) 2017, Marko Viitanen (Fador)

  Permission to use, copy, modify, and/or distribute this software for any purpose 
  with or without fee is hereby granted, provided that the above copyright notice 
  and this permission notice appear in all copies.

  THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES WITH 
  REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY 
  AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT, 
  INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM 
  LOSS OF USE, DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE 
  OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR 
  PERFORMANCE OF THIS SOFTWARE.

*/

#include <c_types.h>
#include <user_interface.h>
#include <mem.h>
#include <osapi.h>
#include <gpio.h>

//From https://github.com/wdim0/esp8266_direct_gpio
#define GPIO5_H         (GPIO_REG_WRITE(GPIO_OUT_W1TS_ADDRESS, 1<<5))
#define GPIO5_L         (GPIO_REG_WRITE(GPIO_OUT_W1TC_ADDRESS, 1<<5))
#define GPIO5(x)        /*GPIO_OUTPUT_SET(5, x)//*/((x)?GPIO5_H:GPIO5_L)

void ws2812_init() { 
  //PIN_FUNC_SELECT(PERIPHS_IO_MUX_MTDO_U, FUNC_GPIO5); 
  PIN_FUNC_SELECT(PERIPHS_IO_MUX_GPIO5_U, FUNC_GPIO5);
  GPIO_OUTPUT_SET(5, 0); //GPIO5 as output low
}

// Send one bit to the WS2812
//  - a bit in WS2812 is a waveform with output set to 1 for x period and then 0 for y period
//   - For 0-bit, output is set to 1 for (according to the specification) 0.35us and 0 for 0.7us
//   - For 1-bit the values are 0.8us 1 and 9.6us 0
//   - The only actual timing that matters is the 0-bit  1-state timing, which here is improvised

static void ws2812_send_bit(uint8_t bit) {  
  
  if(bit) {
    GPIO5(1);
    os_delay_us(1); //1000ns
    GPIO5(0);
    os_delay_us(1); //1000ns
  }
  else {
    GPIO5(1);
    GPIO5(0);
    os_delay_us(1); //1000ns
  }
  
}

// Send one byte (or 8 bits) to the WS2812
// Send MSB first
static void ws2812_send_byte(uint8_t byte) {  
  uint8_t i;
  for(i = 0; i < 8; i++) {
    ws2812_send_bit(byte&128);
    byte <<= 1;
  }
  
}

void ws2812_send_pixel(uint8_t r, uint8_t g, uint8_t b) {
  ws2812_send_byte(g);
  ws2812_send_byte(r);
  ws2812_send_byte(b);  
}

void ws2812_reset() {
  GPIO5(0);
  os_delay_us(50); 
}
