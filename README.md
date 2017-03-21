# Device status indicator system in ESP8266

Runs WS2812 light strip indicating some status info

## Usage

Edit [configure.h](./user/configure.h) to set up device name and WLAN credentials.

Outputs WS2812 control signal via GPIO 2, designed to be used with 16 lights.

Contains an array with 4 stages for each led, stage includes color and delay.

Set and clear lights using \<ESP ip\>:8000/set/\<ID\>/\<state\>/\<r\>/\<g\>/\<b\>/\<delay\> and \<ESP ip\>:8000/clear/\<ID\>, ID being between 1 and 16, state 0-3, r/g/b 0-255 and delay 1-255

Example:

http://\<IP\>:8000/set/1-16/0/20/0/0/100
http://\<IP\>:8000/set/1-16/1/0/20/0/100
http://\<IP\>:8000/set/1-16/2/0/0/20/100



## License

Distributed under ISC license (check LICENSE file)
