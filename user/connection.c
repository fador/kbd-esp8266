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
#include <c_types.h>
#include <user_interface.h>
#include <espconn.h>
#include <mem.h>
#include <osapi.h>
#include <gpio.h>
#include "connection.h"
#include "configure.h"
#include "ws2812_lib.h"


#define GPIO_READ(x)           ((GPIO_REG_READ(GPIO_IN_ADDRESS)&(BIT((x))))!=0)
// Read status
#define GPIO02           GPIO_READ(2)
#define GPIO14           GPIO_READ(14)
#define GPIO13           GPIO_READ(13)
#define GPIO12           GPIO_READ(12)
#define GPIO05           GPIO_READ(5)

LOCAL int cycles_down;

LOCAL int cycles_down_led[8];
LOCAL bool led_pressed[8];

LOCAL ip_addr_t ip;
LOCAL bool output_enabled;

os_timer_t update_light_timer;

LOCAL struct espconn conn2;
LOCAL esp_tcp tcp1;


LOCAL struct espconn conn_udp;
LOCAL esp_udp udp1;
int brightness = 255;

// Some random value
LOCAL uint32_t pseudorandom = 0x31466fa4;


LOCAL uint8_t random_r[] = { 120, 0, 0, 120, 0, 120, 120, 0 };
LOCAL uint8_t random_g[] = { 0, 120, 0, 120, 120, 0, 60, 60 };
LOCAL uint8_t random_b[] = { 0, 0, 120, 0, 120, 120, 0, 120 };


// Control keyboard Mux with GPIO 14,12 and 2
#define SET_KEY_INPUT(x) GPIO_OUTPUT_SET(14, (x)&1); GPIO_OUTPUT_SET(12, ((x)>>1)&1); GPIO_OUTPUT_SET(2, ((x)>>2)&1);


uint32_t ICACHE_FLASH_ATTR esp_random() 
{
	/* Algorithm "xor" from p. 4 of Marsaglia, "Xorshift RNGs" */
	uint32_t x = pseudorandom;
	x ^= x << 13;
	x ^= x >> 17;
	x ^= x << 5;
	pseudorandom = x;
	return x;
}

LOCAL bool output_update;

typedef struct {
  uint8_t r;
  uint8_t g;
  uint8_t b;
} ws2812_pixel;

typedef struct {
  uint8_t states;
  uint8_t current_state;
  uint8_t current_timer;
  ws2812_pixel current_pixel;
  ws2812_pixel pixel[MAX_STATES];
  uint8_t state_time[MAX_STATES];  
  
} ws2812_pixel_states;

typedef struct {
  ws2812_pixel_states n[WS2812_LED_COUNT]; 
} ws2812_pixel_array;

static ws2812_pixel_array* pixels;

static uint8_t setLedValue(uint8_t idx, uint8_t state, uint8_t r, uint8_t g, uint8_t b, uint8_t delay)
{
 
  ws2812_pixel_states *pix = &pixels->n[idx];
  if(pix->states <= state) pix->states = state+1;
  
  pix->current_state = 0;
  pix->current_timer = 0;
  
  pix->pixel[state].r = r;
  pix->pixel[state].g = g;
  pix->pixel[state].b = b;
  pix->state_time[state] = delay;  
  
  pix->current_pixel = pix->pixel[0];
  
  return 1;
}
                            

void ICACHE_FLASH_ATTR update_light(void)
{
  uint8_t i;
   
  // Precalc values
  for(i = 0; i < WS2812_LED_COUNT; i++) {
    ws2812_pixel_states *pix = &pixels->n[i];
    
    // No states -> no light
    if(pix->states == 0) {
      pix->current_pixel.r = 0;
      pix->current_pixel.g = 0;
      pix->current_pixel.b = 0;
    } else {
      if(pix->current_timer == 0) {
        pix->current_pixel = pix->pixel[pix->current_state];
      } else {
        if(pix->current_timer > pix->state_time[pix->current_state]) 
        {
          pix->current_state++;
          if(pix->current_state == pix->states) {
            pix->current_state = 0;
          }
          pix->current_pixel = pix->pixel[pix->current_state];
          pix->current_timer = 0;
        } else {
          float ratio;
          ws2812_pixel orig_pix = pix->pixel[pix->current_state];
          ws2812_pixel next_pix = (pix->current_state+1 == pix->states) ? pix->pixel[0] : pix->pixel[pix->current_state+1];
          
          if(pix->state_time[pix->current_state] != 0) {
            ratio = (float)(pix->current_timer) / (float)(pix->state_time[pix->current_state]);
            pix->current_pixel.r = (uint8_t)((int16_t)orig_pix.r+(int16_t)(ratio*((float)next_pix.r - (float)orig_pix.r)));
            pix->current_pixel.g = (uint8_t)((int16_t)orig_pix.g+(int16_t)(ratio*((float)next_pix.g - (float)orig_pix.g)));
            pix->current_pixel.b = (uint8_t)((int16_t)orig_pix.b+(int16_t)(ratio*((float)next_pix.b - (float)orig_pix.b)));
          }
        }
      }
      
      pix->current_timer++;
    }
  }
  
    
  
  for(i = 0; i < 8; i++) {
    SET_KEY_INPUT(i);
    os_delay_us(1); //1000ns
    uint8_t key_input = GPIO13;
    
    if(!key_input) {
      cycles_down_led[i]++;
    } else {
      led_pressed[i] = false;
      cycles_down_led[i] = 0;
    }

    if(cycles_down_led[i] > 1 && !led_pressed[i])  {
      led_pressed[i] = true;
      int val = esp_random()&0xf;
      setLedValue(i, 0, random_r[val], random_g[val], random_b[val], 20);
    }
  }


  // Disable interrupts while sending, timing critical step  
  system_soft_wdt_stop();
  ets_intr_lock();
  ws2812_reset(DEFAULT_LED_PORT);
  for(i = 0; i < WS2812_LED_COUNT; i++) {
    ws2812_pixel_states *pix = &pixels->n[i];
    ws2812_send_pixel(DEFAULT_LED_PORT, pix->current_pixel.r, pix->current_pixel.g, pix->current_pixel.b);
  }
  ets_intr_unlock();
  system_soft_wdt_restart();

}


void ICACHE_FLASH_ATTR serverInit() {
  
  int i; 
  
  output_enabled = true;
  output_update = true;
  
  for(i = 0; i < 8; i++) {
    cycles_down_led[i] = 0;
    led_pressed[i] = false;
  }
  
  pixels = (ws2812_pixel_array*)os_malloc(sizeof(ws2812_pixel_array));
  os_memset(pixels,0, sizeof(ws2812_pixel_array)); 
  
  os_timer_disarm(&update_light_timer);
  os_timer_setfn(&update_light_timer, (os_timer_func_t *)update_light, NULL);
  os_timer_arm(&update_light_timer, 30, 1);

  setLedValue(0, 0, 120, 0, 0, 20);
  setLedValue(1, 0, 0, 120, 0, 20);
  setLedValue(2, 0, 0, 0, 120, 20);
  setLedValue(3, 0, 120, 120, 0, 20);
  setLedValue(4, 0, 0, 120, 120, 20);
  setLedValue(5, 0, 120, 0, 120, 20);
  setLedValue(6, 0, 120, 60, 0, 20);
  setLedValue(7, 0, 0, 60, 120, 20);
  
  setLedValue(0, 1, 0, 0, 0, 20);
  setLedValue(1, 1, 0, 0, 0, 20);
  setLedValue(2, 1, 0, 0, 0, 20);
  setLedValue(3, 1, 0, 0, 0, 20);
  setLedValue(4, 1, 0, 0, 0, 20);
  setLedValue(5, 1, 0, 0, 0, 20);
  setLedValue(6, 1, 0, 0, 0, 20);
  setLedValue(7, 1, 0, 0, 0, 20);
}
