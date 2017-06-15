# GoodWe Solar inverter logger based on ESP8266

This ESP8266 firmware enables you to read information from a GoodWe solar inverter through it's RS485 bus.
Sending information to a MQTT broker is supported, as well as uploading information to [PVoutput](https://pvoutput.org/).

## Requirements
  - GoodWe inverter with RS485 connector (this software has been tested with GW3600D-NS and GW3000-NS inverter, but others might also work)
  - RS485 converter (can be found on sites like AliExpress, search for *SCM TTL to RS485 Adapter 485 to UART Serial Port 3.3V 5V Level Converter Module*)  
  - ESP8266 (like NodeMCU or Wemos D1 mini)
  - Computer with Arduino IDE installed

## Flashing firmware
 - To flash this firmware, you will need to install the Arduino IDE and configure it for the board you are using (NodeMCU / Wemos)
 - You will also need the following libraries
   - ['Time' library](https://github.com/PaulStoffregen/Time)
   - [NTPClient](https://github.com/arduino-libraries/NTPClient)
   - [PubSubClient](https://github.com/knolleary/pubsubclient)
   Place these into the `libraries` folder belonging to Arduino on your system.
 - Clone/download this repository
 - Rename the `Settings.example.h` to `Settings.h` and configure it to match your preferred settings
 - Compile and upload the firmware to your ESP8266
 - Diagnostics information is sent over serial at 115200 baud *(wifi status, MQTT status, inverter connection status)*

## Connecting hardware
The RS485 connector on the inverter is located at the bottom of the inverter, visible when you remove the WiFi module. It can also be hidden behind a small metal plate (for inverters without WiFi). It is a 6 pin green screw terminal (female) you can order the male counterpart on eBay/AliExpress/Farnell. Using female DuPont wires that you insert directly into the connector might also work.

***Above information applies to the GWxxxxD-NS range of inverters. Other models might use a different method of connecting. Consult your inverter manual.***

The connector inside the inverter looks like this:

    +--+--+--+--+--+--+    +------------+
    |  |  |  |  |  |  |    |------------|   
    +--+--+--+--+--+--+    +------------+     
     1  2  3  4  5  6           USB           

Connection scheme:

Inverter | RS485 converter
--- | ---
pin 1  (485_TX-) | A+
pin 2 (485_TX+) | B-

**Pay attention, RS485 will only work if you disconnect the WiFi module and restart the inverter. 
They cannot work simultaneously.**

Connect the RS485 converter to your ESP8266 like this:

RS485 converter | ESP8266
--- | ---
GND | G / GND
RXD | D1
TXD | D2
VCC | 5V / 3V3

*(`D1` (receive) and `D2` (transmit) can be configured to different pins in `Settings.h`)*. It might look weird to connect `RXD` of the module to the receive pin of the ESP8266, but this is how the RS485 converter is labeled.

## MQTT
Subscribe to the `goodwe/` topic in your MQTT client. Information will be posted there and will look like this:
```
goodwe/93600DVA295R148/vpv1 242.6
goodwe/93600DVA295R148/vpv2 235.7
goodwe/93600DVA295R148/ipv1 4.9
goodwe/93600DVA295R148/ipv2 5.5
goodwe/93600DVA295R148/vac1 240.6
goodwe/93600DVA295R148/iac1 10.7
goodwe/93600DVA295R148/fac1 49.99
goodwe/93600DVA295R148/pac 2582
goodwe/93600DVA295R148/temp 38.6
goodwe/93600DVA295R148/eday 6.40
goodwe/93600DVA295R148/workmode 1
goodwe/93600DVA295R148/online 1
```
Field | Description | Unit
--- | --- | ---
vpv1 | Voltage of first string of solarpanels | V
vpv2 | Voltage of second string of solarpanels | V
ipv1 | Current of first string of solarpanels | A
ipv2 | Current of second string of solarpanels | A
vac1 | Voltage mains side inverter | V
iac1 | Current mains side inverter | A
fac1 | Frequency (Hz) mains side inverter | Hz
pac | Current power production in Watt | W
temp | Internal temperature of inverter | &deg;C
eday | Energy produced today | kWh
workmode | Undocumented parameter. Default=1 | binary
online | Inverter status (1=on, 0=off) | binary

## PVoutput
When you have your PVoutput *API key* and *System ID* configured correctly in `Settings.h`, production data from the inverter will be uploaded to PVoutput every 5 minutes *(interval is configurable in `Settings.h`, but don't go lower than the minimal interval of every 5 minutes as specified by PVoutput)*.
When multiple inverters are connected, by daisy-chaining the RS485 cable, only the production data of the first inverter will be uploaded.

For the PVoutput upload function to work, it is important that the ESP8266 has access to the internet. 
Apart from connections being made to PVoutput, you will also see that the ESP8266 talks with pool.ntp.org on a regular interval. This is done to retrieve the current time, which is needed to post data to PVoutput.

If you plan to use only MQTT, internet access for the ESP8266 is not needed.


## TODO
- Webpage to configure parameters that are now hardcoded in `Settings.h`
- Extract other parameters from inverter(s)
- Detect if inverter is single phase or three phase model
