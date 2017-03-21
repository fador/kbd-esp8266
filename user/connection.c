/*
  Copyright (c) 2016, Marko Viitanen (Fador)

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
#include "connection.h"
#include "configure.h"
#include "ws2812_lib.h"

os_timer_t update_light_timer;

LOCAL ip_addr_t ip;

LOCAL struct espconn conn2;
LOCAL esp_tcp tcp1;

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

ws2812_pixel_array* pixels;


#define HTTP_HEADER(CONTENT_TYPE) "HTTP/1.1 200 OK\r\n" \
                                  "Content-Type: " CONTENT_TYPE "\r\n" \
                                  "Cache-Control: no-cache, no-store, must-revalidate\r\n" \
                                  "Pragma: no-cache\r\n" \
                                  "Expires: 0\r\n" \
                                 "Content-Length: %d\r\n" \
                                 "Server: Unspecified, UPnP/1.0, Unspecified\r\n" \
                                 "connection: close\r\n" \
                                 "Last-Modified: Sat, 01 Jan 2000 00:00:00 GMT\r\n" \
                                 "\r\n"
                                 
                            
                            
static uint8_t readValue(char* buf, uint8_t* in_offset) {
  uint8_t value = 0;
  uint8_t offset = 0;
  while((uint8_t)(buf[offset]-'0') < 10) {
    value *= 10;
    value += buf[offset]-'0';
    offset++;
  }
  
  *in_offset += offset;
  
  return value;
}

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
                            

LOCAL void ICACHE_FLASH_ATTR webserver_recv(void *arg, char *data, unsigned short length)
{
    struct espconn *ptrespconn = arg;
    char buffer[BUFFERLEN];
    char message[100];
    // Return settings when requesting setup.xml
    if(os_strstr(data,"set.php")) {
      
      os_sprintf(message, "Test response");
      
      // Append the payload to the HTTP headers.
      os_sprintf(buffer, HTTP_HEADER("text/html")
                         "%s", os_strlen(message), message); 
                  
      espconn_send(ptrespconn, buffer, os_strlen(buffer));
      return;    
    } else if(os_strstr(data, "set/")) {   
      char* datapos = (char*)os_strstr(data, "set/");
      uint8_t offset = 0;
      datapos += 4;
      
      uint8_t led_idx = readValue(&datapos[offset], &offset);
      uint8_t led_range = 0;
      
      if(datapos[offset] == '-') {
        offset++;
        led_range = readValue(&datapos[offset], &offset);        
      }
      uint8_t led_state = 0;
      uint8_t led_r = 0;
      uint8_t led_g = 0;
      uint8_t led_b = 0;
      uint8_t led_delay = 0;
      uint8_t error = 1;
      
      led_idx--;
      if(led_range) led_range--;
      
      // Verify that the led index exists
      if(led_idx < WS2812_LED_COUNT && led_range < WS2812_LED_COUNT) {        

        if(datapos[offset] == '/') {
          offset++;
          led_state = readValue(&datapos[offset], &offset);
          if(led_state < MAX_STATES && datapos[offset] == '/') {
            offset++;
            led_r = readValue(&datapos[offset], &offset);
            if(datapos[offset] == '/') {
              offset++;
              led_g = readValue(&datapos[offset], &offset);
              if(datapos[offset] == '/') {
                offset++;
                led_b = readValue(&datapos[offset], &offset);
                 if(datapos[offset] == '/') {
                  offset++;
                  led_delay = readValue(&datapos[offset], &offset);
                  if(led_delay != 0) {
                    setLedValue(led_idx,led_state, led_r,led_g,led_b,led_delay);
                    if(led_range) {
                      int i;
                      for(i = led_idx+1; i <= led_range; i++) {
                        setLedValue(i,led_state, led_r,led_g,led_b,led_delay);
                      }
                    }
                    
                    error = 0;
                  }
                }
              }
            }
          }
        }
        if(error) {
          os_sprintf(message, "Expecting /set/<id>/<state>/<r>/<g>/<b>/<delay>");
        } else {
          os_sprintf(message, "Value set ID: %d state %d R %d G %d B %d Delay %d", led_idx, led_state, led_r, led_g, led_b, led_delay);
        }
      } else {
        os_sprintf(message, "ID value out of range");
      }
      
      // Append the payload to the HTTP headers.
      os_sprintf(buffer, HTTP_HEADER("text/html")
                         "%s", os_strlen(message), message); 
                  
      espconn_send(ptrespconn, buffer, os_strlen(buffer));
    } else if(os_strstr(data, "clear/")) {   
      char* datapos = (char*)os_strstr(data, "clear/");
      datapos += 6;
      
      char value = datapos[0]-'0';
      
      value--;
      
      if(value < WS2812_LED_COUNT) {        
        
        os_memset(&pixels->n[value],0, sizeof(ws2812_pixel_states));
        
        os_sprintf(message, "Value cleared");
      } else {
        os_sprintf(message, "Value out of range");
      }
      
      // Append the payload to the HTTP headers.
      os_sprintf(buffer, HTTP_HEADER("text/html")
                         "%s", os_strlen(message), message); 
      espconn_send(ptrespconn, buffer, os_strlen(buffer));
    } else {
      os_sprintf(buffer, HTTP_HEADER("text/plain charset=\"utf-8\"")
                         "ok", 2);                        
      espconn_send(ptrespconn, buffer, os_strlen(buffer));
    }
    
}

LOCAL void ICACHE_FLASH_ATTR webserver_listen(void *arg)
{
    struct espconn *pesp_conn = arg;

    espconn_regist_recvcb(pesp_conn, webserver_recv);
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
  
  // Disable interrupts while sending, timing critical step
  ets_intr_lock();
  ws2812_reset();
  for(i = 0; i < 16; i++) {
    ws2812_pixel_states *pix = &pixels->n[i];
    ws2812_send_pixel(pix->current_pixel.r, pix->current_pixel.g, pix->current_pixel.b);
  }
  ets_intr_unlock();

}

void ICACHE_FLASH_ATTR serverInit() {
  
  os_printf("Free heap: %d\n", system_get_free_heap_size());
  
  
  // Set up the TCP server
  tcp1.local_port = 8000;
  conn2.type = ESPCONN_TCP;
  conn2.state = ESPCONN_NONE;
  conn2.proto.tcp = &tcp1;
  
  espconn_regist_connectcb(&conn2, webserver_listen);
  espconn_accept(&conn2);
  
  pixels = (ws2812_pixel_array*)os_malloc(sizeof(ws2812_pixel_array));
  os_memset(pixels,0, sizeof(ws2812_pixel_array));

  os_timer_disarm(&update_light_timer);
  os_timer_setfn(&update_light_timer, (os_timer_func_t *)update_light, NULL);
  os_timer_arm(&update_light_timer, 30, 1);
  

}
