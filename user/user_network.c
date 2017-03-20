/*
Tnx to Sprite_TM (source came from his esp8266ircbot)
*/

#include <c_types.h>
#include <user_interface.h>
#include <espconn.h>
#include <mem.h>
#include <osapi.h>
#include "user_network.h"
#include "user_config.h"
#include "connection.h"
#include "ws2812_lib.h"
#include "configure.h"

LOCAL os_timer_t network_timer;

uint8_t cred_index = 0;

void ICACHE_FLASH_ATTR network_start() {
  serverInit();
}

void ICACHE_FLASH_ATTR network_update_credential(char* ssid, char* password) 
{
  struct station_config stationConf;
  
  os_memcpy(&stationConf.ssid, ssid, 32);
  os_memcpy(&stationConf.password, password, 32);

  wifi_set_opmode( STATION_MODE );
  wifi_set_phy_mode(PHY_MODE_11N);
  wifi_station_set_config(&stationConf);
  wifi_station_connect();
}

void ICACHE_FLASH_ATTR network_check_ip(void)
{
  struct ip_info ipconfig;

  os_timer_disarm(&network_timer);

  wifi_get_ip_info(STATION_IF, &ipconfig);


  if (wifi_station_get_connect_status() == STATION_GOT_IP && ipconfig.ip.addr != 0) {
    char page_buffer[20];
    int i;
    os_sprintf(page_buffer,"IP: %d.%d.%d.%d",IP2STR(&ipconfig.ip));
    network_start();
    
    // Light up green leds
    ets_intr_lock();
    ws2812_reset();
    for(i = 0; i < 16; i++) {
      ws2812_send_pixel(0, (i < ((cred_index%WLAN_CREDENTIALS)+1)<<1)?50:0, 0);
    }
    ets_intr_unlock();
  }
  else 
  {
    int i;
    os_printf("No ip found\n\r");
    os_timer_setfn(&network_timer, (os_timer_func_t *)network_check_ip, NULL);
    os_timer_arm(&network_timer, 5000, 0);
    
    // Use next credential on the list
    cred_index++;

    os_printf("Using SSID %s\n\r",WLAN_SSID_LIST[cred_index%WLAN_CREDENTIALS]);
    network_update_credential(WLAN_SSID_LIST[cred_index%WLAN_CREDENTIALS], WLAN_PASSWD_LIST[cred_index%WLAN_CREDENTIALS]);
    
    // Light up two red leds per credential index for indication which is being tested
    ets_intr_lock();
    ws2812_reset();
    for(i = 0; i < 16; i++) {
      ws2812_send_pixel((i < ((cred_index%WLAN_CREDENTIALS)+1)<<1)?50:0, 0, 0);
    }
    ets_intr_unlock();
    
  } 
        
}

void ICACHE_FLASH_ATTR network_init()
{
    os_printf("Network init\n");
    os_timer_disarm(&network_timer);
    os_timer_setfn(&network_timer, (os_timer_func_t *)network_check_ip, NULL);
    os_timer_arm(&network_timer, 5000, 0);    
}
