/*
   MKR1000-DHT22-MQTT.ino
*/

#include "config.h"
#include <WiFi101.h>
#include <WiFiClient.h>
#include <RTCZero.h>
#include <DHT.h>
#include <Adafruit_MQTT.h>
#include <Adafruit_MQTT_Client.h>
#include <Adafruit_SleepyDog.h>

#define DHTPIN 2        // what digital pin we're connected to
#define DHTTYPE DHT22   // DHT 11 is also possible

#define TEMPERATURE_TOPIC "kitchen/sensors/temperatureout"
#define HUMIDITY_TOPIC "kitchen/sensors/humidityout"

DHT dht(DHTPIN, DHTTYPE);
RTCZero rtc;

// Set some dummy data, since we just want intervals.
const byte seconds = 0;
const byte minutes = 0;
const byte hours = 0;
const byte day = 1;
const byte month = 1;
const byte year = 17;

WiFiClient wifiClient;
Adafruit_MQTT_Client mqtt(&wifiClient, MQTT_SERVER, MQTT_SERVERPORT, MQTT_USERNAME, MQTT_PASSWORD);
Adafruit_MQTT_Publish temperatureFeed = Adafruit_MQTT_Publish(&mqtt, TEMPERATURE_TOPIC, MQTT_QOS_1);
Adafruit_MQTT_Publish humidityFeed = Adafruit_MQTT_Publish(&mqtt, HUMIDITY_TOPIC, MQTT_QOS_1);

float temperature, humidity;

bool measureTrigger = false;

void getNextSample(float* Temperature, float* Humidity)
{
  *Humidity = dht.readHumidity();
  *Temperature = dht.readTemperature();
}

void connectWifi()
{
  WiFi.begin(WIFI_SSID, WIFI_PASS);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
  }
}

void disconnectWifi()
{
  WiFi.disconnect(); // Try to repair with disconnect to Wifi
  delay(500);

  while (WiFi.status() == WL_CONNECTED)
  {
    delay(250);
    WiFi.disconnect(); // Try to repair with disconnect to Wifi
    if (WiFi.status() == WL_CONNECTED)
    {
      WiFi.end();
    }
  }
}

void connectMQTT()
{
  int8_t ret;
  if (mqtt.connected()) {
    return;
  }

  while ((ret = mqtt.connect()) != 0) {
    mqtt.disconnect();
  }
}

void disconnectMQTT()
{
  mqtt.disconnect();
}

void setup() {
  WiFi.hostname("OutsideThermometer");

  pinMode(6, OUTPUT);
  dht.begin();

  // Set the RTC
  rtc.begin();
  rtc.setTime(hours, minutes, seconds);
  rtc.setDate(day, month, year);
  rtc.setAlarmTime(00, 00, 00);
  rtc.enableAlarm(rtc.MATCH_MMSS);
  rtc.attachInterrupt(alarmInterrupt); // attach the interrupt
  rtc.standbyMode();
}

void alarmInterrupt() {
  measureTrigger = true; // we got a match
}

void messageReceived(String topic, String payload, char * bytes, unsigned int length) {}

void work()
{
  Watchdog.reset();
  getNextSample(&temperature, &humidity);
  Watchdog.reset();

  connectWifi();
  connectMQTT();
  digitalWrite(6, LOW);

  Watchdog.reset();

  temperatureFeed.publish(temperature);
  humidityFeed.publish(humidity);

  Watchdog.reset();

  disconnectMQTT();
  disconnectWifi();

}

void loop() {
  digitalWrite(6, HIGH);
  Watchdog.enable(8000);
  if (measureTrigger) {
    work(); // do the work
    measureTrigger = false;
    int alarmMinutes = rtc.getMinutes();
    alarmMinutes += 1;
    if (alarmMinutes >= 60) {
      alarmMinutes -= 60;
    }
    rtc.setAlarmTime(rtc.getHours(), alarmMinutes, rtc.getSeconds());
  }
  Watchdog.disable();

  rtc.standbyMode();
}
