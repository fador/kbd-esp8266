# Device status indicator system in ESP8266

Runs WS2812 light strip indicating some status info

## Usage

Edit [configure.h](./user/configure.h) to set up device name and WLAN credentials.

Outputs WS2812 control signal via GPIO 2, designed to be used with 16 lights.

Contains a status array for 4 values, each which affects the color of 4 lights.

Set and clear lights using \<ESP ip\>:8000/set/\<ID\> and \<ESP ip\>:8000/clear/\<ID\>, ID being between 1 and 4

## License

Distributed under ISC license (check LICENSE file)
