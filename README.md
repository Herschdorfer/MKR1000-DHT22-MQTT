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

Additionally a Watchdog timer with a 70 seconds timeout resets the MKR1000 if it hangs because of connection issues.
