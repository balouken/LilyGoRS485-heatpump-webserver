# LillyGoRS485-heatpump-webserver

Adjust parameters on a heat pump with RS485.

Possible on webpage:
- adjust heat curve temperatures
- adjust hot water tank temperatures for smart grid

With a digital input, the setting for smart grid can be set:
- Low tank temperature when contact is open
- High tank temperature when contact is closed

All settings are saved in separated files on ESP32 (SPIFFS), when restarting the ESP32, settings are not lost.

![image](https://user-images.githubusercontent.com/56886296/198827664-eb10d15b-9697-4ae7-b00c-6b64ae5a8220.png)
