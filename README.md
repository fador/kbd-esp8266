# Device status indicator system in ESP8266

Runs WS2812 light strip indicating some status info

## Usage

Edit [configure.c](./user/configure.c) to set up WLAN credentials.

Outputs WS2812 control signal via GPIO 2, designed to be used with 16 lights.

Contains an array with 4 stages for each led, stage includes color and delay.

Set and clear lights using \<ESP ip\>:8000/set/\<ID\>/\<state\>/\<r\>/\<g\>/\<b\>/\<delay\> and \<ESP ip\>:8000/clear/\<ID\>, ID being between 1 and 16, state 0-3, r/g/b 0-255 and delay 1-255

### Example:

http://192.168.0.100:8000/set/1-16/0/20/0/0/100  
http://192.168.0.100:8000/set/1-16/1/0/20/0/100  
http://192.168.0.100:8000/set/1-16/2/0/0/20/100

### Video:

Using  
http://192.168.0.100:8000/set/1-16/0/30/10/10/20  
http://192.168.0.100:8000/set/1-16/1/0/0/0/20  
http://192.168.0.100:8000/set/1-16/2/20/0/60/20  
http://192.168.0.100:8000/set/1-16/3/0/0/0/20  

[![Driving ES2812 LED ring using ESP8266](https://img.youtube.com/vi/8O8i_OS0uOs/0.jpg)](https://www.youtube.com/watch?v=8O8i_OS0uOs)



## License

Distributed under ISC license (check LICENSE file)
