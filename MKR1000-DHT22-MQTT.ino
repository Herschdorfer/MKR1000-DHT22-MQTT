
#include "config.h"
#include <WiFi101.h>
#include <RTCZero.h>
#include <DHT.h>
#include <Adafruit_MQTT.h>
#include <Adafruit_MQTT_Client.h>
#include <Adafruit_SleepyDog.h>

#define DHTPIN 2     // what digital pin we're connected to
#define DHTTYPE DHT22   // DHT 11

DHT dht(DHTPIN, DHTTYPE);

RTCZero rtc;

/* Change these values to set the current initial time */
const byte seconds = 0;
const byte minutes = 0;
const byte hours = 0;

/* Change these values to set the current initial date */
const byte day = 1;
const byte month = 1;
const byte year = 17;

WiFiClient wifiClient;
Adafruit_MQTT_Client mqtt(&wifiClient, MQTT_SERVER, MQTT_SERVERPORT, MQTT_USERNAME, MQTT_USERNAME);
Adafruit_MQTT_Publish temperatureFeed = Adafruit_MQTT_Publish(&mqtt, "kitchen/sensors/temperatureout");
Adafruit_MQTT_Publish humidityFeed = Adafruit_MQTT_Publish(&mqtt, "kitchen/sensors/humidityout");

float temperature, humidity;

bool measureTrigger = false;

void getNextSample(float* Temperature, float* Humidity)
{
  delay(2000);
  *Humidity = dht.readHumidity();
  *Temperature = dht.readTemperature();
  Serial.print("Temperature="); Serial.println(*Temperature);
  Serial.print("Humidity="); Serial.println(*Humidity);
}

void connectWifi()
{
  Serial.println("Connect Wifi");
  WiFi.begin(WIFI_SSID, WIFI_PASS);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("Connected Wifi");
}

void disconnectWifi()
{
  Serial.println("Disconnecting Wifi");
  WiFi.end();
  Serial.println("Disconnect Wifi");
}

void connectMQTT()
{
  int8_t ret;
  if (mqtt.connected()) {
    return;
  }

  Serial.print("Connecting to MQTT... ");

  while ((ret = mqtt.connect()) != 0) { 
    Serial.println(mqtt.connectErrorString(ret));
    Serial.println("Retrying MQTT connection in 5 seconds...");
    mqtt.disconnect();
    delay(5000);  // wait 5 seconds
  }
  Serial.println("MQTT Connected!");
}

void disconnectMQTT()
{
  mqtt.disconnect();
}

void setup() {
  pinMode(6, OUTPUT);

  Serial.begin(57600);
  dht.begin();

  Watchdog.enable(70000);

  rtc.begin();
  rtc.setTime(hours, minutes, seconds);
  rtc.setDate(day, month, year);
  rtc.setAlarmTime(00, 00, 00);
  rtc.enableAlarm(rtc.MATCH_MMSS);
  rtc.attachInterrupt(alarmInterrupt);
  rtc.standbyMode();

}

void messageReceived(String topic, String payload, char * bytes, unsigned int length) {}

void alarmInterrupt() {
  measureTrigger = true;
}

void work()
{
  digitalWrite(6, HIGH);
  getNextSample(&temperature, &humidity);

  connectWifi();
  connectMQTT();

  delay(1000);

  Serial.println("Publishing..."); Serial.println();
  temperatureFeed.publish(temperature);
  humidityFeed.publish(humidity);

  delay(1000);
  disconnectMQTT();
  disconnectWifi();
  digitalWrite(6, LOW);
}

void loop() {
  if (measureTrigger) {
    work();
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
