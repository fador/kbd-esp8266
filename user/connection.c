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


LOCAL ip_addr_t ip;

LOCAL struct espconn conn2;
LOCAL esp_tcp tcp1;


#define HTTP_HEADER(CONTENT_TYPE) "HTTP/1.1 200 OK\r\n" \
                                  "Content-Type: " CONTENT_TYPE "\r\n" \
                                 "Content-Length: %d\r\n" \
                                 "Server: Unspecified, UPnP/1.0, Unspecified\r\n" \
                                 "connection: close\r\n" \
                                 "Last-Modified: Sat, 01 Jan 2000 00:00:00 GMT\r\n" \
                                 "\r\n"
                                 
                            

LOCAL void ICACHE_FLASH_ATTR webserver_recv(void *arg, char *data, unsigned short length)
{
    struct espconn *ptrespconn = arg;
    char buffer[BUFFERLEN];
    
    // Return settings when requesting setup.xml
    if(os_strstr(data,"setup.xml")) {
      
      char xml_message[500];
      
      // Define the payload part
      const char setup_xml[] = "<?xml version=\"1.0\"?>\r\n"
                        "<root>\r\n"
                        "  <device>\r\n"
                        "    <deviceType>urn:FadorTest:device:controllee:1</deviceType>\r\n"
                        "    <friendlyName>%s</friendlyName>\r\n"
                        "    <manufacturer>Belkin International Inc.</manufacturer>\r\n"
                        "    <modelName>Emulated Socket</modelName>\r\n"
                        "    <modelNumber>3.1415</modelNumber>\r\n"
                        "    <UDN>uuid:Socket-1_0-%s</UDN>\r\n"
                        "  </device>\r\n"
                        "</root>";
                        
      // Fill in the payload with the correct settings, we need to get the size of this later
      os_sprintf(xml_message,setup_xml, DEVICE_NAME, SERIAL_NUMBER);
      
      // Append the payload to the HTTP headers.
      os_sprintf(buffer, HTTP_HEADER("text/xml")
                         "%s", os_strlen(xml_message), xml_message); 
                  
      espconn_send(ptrespconn, buffer, os_strlen(buffer));
      return;    
      
    } else if(os_strstr(data, "basicevent1")) { // Getting basicevent1, turning the switch on/off
      
      char message[500];
      
      // Simply search for the binarystate, if it's found, we enable the "switch"
      if(os_strstr(data, "<BinaryState>1</BinaryState>")) {
        os_sprintf(message, returnState, 1);
        os_printf("Turning on!\n");
        
        /* ToDo:
          DO STUFF WHEN ENABLING THE DEVICE
        */
        
      } else {
        os_sprintf(message, returnState, 0);
        os_printf("Turning off!\n");
        
        /* ToDo:
          DO STUFF WHEN DISABLING THE DEVICE
        */
      }
      
      
      os_sprintf(buffer, HTTP_HEADER("text/plain charset=\"utf-8\"")
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

void ICACHE_FLASH_ATTR serverInit() {
  
  os_printf("Free heap: %d\n", system_get_free_heap_size());
  
  
  // Set up the TCP server
  tcp1.local_port = 8000;
  conn2.type = ESPCONN_TCP;
  conn2.state = ESPCONN_NONE;
  conn2.proto.tcp = &tcp1;
  
  espconn_regist_connectcb(&conn2, webserver_listen);
  espconn_accept(&conn2);

}
