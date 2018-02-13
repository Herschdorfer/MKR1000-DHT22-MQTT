# MKR1000-DHT22-MQTT

This project enables a MKR1000 to connect to a MQTT broker and send data from an attached DHT22 or DHT11.

The MKR1000 will
- Read the temperature and humidity
- Connect to Wifi
- Connect to MQTT
- Send Data
- Disconnect MQTT
- Disconnect Wifi
- Sleep for 1 minute

*Additionally a Watchdog timer with the maximum of 8 seconds resets the MKR1000 if it hangs because of connection issues.*

# How to use
Create a config.h file with the following contents
```
#define WIFI_SSID "YOUR_SSID"
#define WIFI_PASS "YOUR_WIFI_PASSWORD"

#define MQTT_SERVER "YOUR_MQTT DNS/IP"
#define MQTT_SERVERPORT 1883
#define MQTT_USERNAME "YOUR_MQTT_USERNAME"
#define MQTT_USERNAME "YOUR_MQTT_PASSWORD"
```

The just burn the the program.

**IMPORTANT**
When you want to flash the MKR1000 for the second time, this will not work unless you press the RESET button twice and change the COM Port.
The board is in sleep mode between the send intervals and will not answer to command via USB unless you double press the RESET button.

# needed libraries

RTC Zero
Sleepy Dog
