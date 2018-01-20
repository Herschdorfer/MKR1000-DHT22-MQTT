/*
 * MKR1000-DHT22-MQTT.ino
 *  
 *  
 */


#include "config.h"
#include <WiFi101.h>
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
Adafruit_MQTT_Publish temperatureFeed = Adafruit_MQTT_Publish(&mqtt, TEMPERATURE_TOPIC);
Adafruit_MQTT_Publish humidityFeed = Adafruit_MQTT_Publish(&mqtt, HUMIDITY_TOPIC);

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
  WiFi.end();
}

void connectMQTT()
{
  int8_t ret;
  if (mqtt.connected()) {
    return;
  }

  while ((ret = mqtt.connect()) != 0) { 
    mqtt.disconnect();
    delay(5000);  // wait 5 seconds
  }
}

void disconnectMQTT()
{
  mqtt.disconnect();
}

void setup() {
  pinMode(6, OUTPUT);
  dht.begin();

  // Timeout are 70 seconds since we should send every 60 seconds and reset the timer.
  Watchdog.enable(70000);

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
  digitalWrite(6, HIGH);
  getNextSample(&temperature, &humidity);

  connectWifi();
  connectMQTT();

  temperatureFeed.publish(temperature);
  humidityFeed.publish(humidity);

  disconnectMQTT();
  disconnectWifi();
  digitalWrite(6, LOW);
}

void loop() {
  if (measureTrigger) {
    work(); // to the work
    measureTrigger = false;
    int alarmMinutes = rtc.getMinutes();
    alarmMinutes += 1;
    if (alarmMinutes >= 60) {
      alarmMinutes -= 60;
    }
    rtc.setAlarmTime(rtc.getHours(), alarmMinutes, rtc.getSeconds());
    Watchdog.reset();
  }
  rtc.standbyMode();
}
