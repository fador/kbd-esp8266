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

LOCAL os_timer_t network_timer;

void ICACHE_FLASH_ATTR network_start() {
  serverInit();
}

void ICACHE_FLASH_ATTR network_check_ip(void)
{
  struct ip_info ipconfig;

  os_timer_disarm(&network_timer);

  wifi_get_ip_info(STATION_IF, &ipconfig);


  if (wifi_station_get_connect_status() == STATION_GOT_IP && ipconfig.ip.addr != 0) {
    char page_buffer[20];
    os_sprintf(page_buffer,"IP: %d.%d.%d.%d",IP2STR(&ipconfig.ip));
    network_start();
  }
  else 
  {
    os_printf("No ip found\n\r");
    os_timer_setfn(&network_timer, (os_timer_func_t *)network_check_ip, NULL);
    os_timer_arm(&network_timer, 1000, 0);
  } 
        
}

void ICACHE_FLASH_ATTR network_init()
{
    os_printf("Network init\n");
    os_timer_disarm(&network_timer);
    os_timer_setfn(&network_timer, (os_timer_func_t *)network_check_ip, NULL);
    os_timer_arm(&network_timer, 1000, 0);    
}
