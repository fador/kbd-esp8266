/*
  Copyright (c) 2018, Marko Viitanen (Fador)

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

//From https://github.com/wdim0/esp8266_direct_gpio
#define GPIO_H(x)         (GPIO_REG_WRITE(GPIO_OUT_W1TS_ADDRESS, 1<<(x)))
#define GPIO_L(x)         (GPIO_REG_WRITE(GPIO_OUT_W1TC_ADDRESS, 1<<(x)))
#define GPIO(port,val)        ((val)?GPIO_H(port):GPIO_L(port))

#define DEFAULT_LED_PORT 15

#ifndef PERIPHS_IO_MUX_GPIO12_U
  #define PERIPHS_IO_MUX_GPIO12_U          (PERIPHS_IO_MUX + 0x4)
#endif

#ifndef PERIPHS_IO_MUX_GPIO13_U
  #define PERIPHS_IO_MUX_GPIO13_U          (PERIPHS_IO_MUX + 0x8)
#endif

#ifndef PERIPHS_IO_MUX_GPIO14_U
  #define PERIPHS_IO_MUX_GPIO14_U          (PERIPHS_IO_MUX + 0x0C)
#endif

#ifndef PERIPHS_IO_MUX_GPIO15_U
  #define PERIPHS_IO_MUX_GPIO15_U          (PERIPHS_IO_MUX + 0x10)
#endif


void ws2812_init(uint8_t port);
void ws2812_send_pixel(uint8_t port, uint8_t r, uint8_t g, uint8_t b);
void ws2812_reset(uint8_t port);