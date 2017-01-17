# hios-environment-esp8266

Using this project as sort of a guinea pig to implement IOT features which can be used in other devices

Final product is a device for recording and reporting on room environment information, maybe a thermostat with touchscreen

**Implemented:**
* Simple flash based file structure
* basic http server serving files from file system
* simple flash OTA upgrade
* AP mode fallback if station connect fails

**TODO**
* Http API for configuration changes or status monitoring
* LCD integration
* BME280 driver
* (maybe) websocket support for realtime feeds
  
Filesystem blob is created using this tool:
https://github.com/efess/esp-binfs

Using Tuan PM's wonderful MQTT lib:
https://github.com/tuanpmt/esp_mqtt

OTA bootloader by Tuan PM as well:
https://github.com/tuanpmt/esp-bootloader
