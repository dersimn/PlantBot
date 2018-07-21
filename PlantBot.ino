#include "default_config.h"
#include "handy_functions.h"

#include <ESP8266WiFi.h>

#include <GenericLog.h>         // https://github.com/dersimn/ArduinoUnifiedLog
#include <NamedLog.h>
#include <LogHandler.h>
#include <LogSerialModule.h>

#include <Thread.h>             // https://github.com/ivanseidel/ArduinoThread
#include <ThreadController.h>
#include <ThreadRunOnce.h>      // https://github.com/dersimn/ArduinoThreadRunOnce

#include <PubSubClient.h>       // https://github.com/knolleary/pubsubclient
#include <PubSubClientTools.h>  // https://github.com/dersimn/ArduinoPubSubClientTools

#include <ArduinoJson.h>

#include <ArduinoOTA.h>
#include <ESP8266mDNS.h>
#include <WiFiUdp.h>

#include <OneWire.h>
#include <DallasTemperature.h>  // https://github.com/milesburton/Arduino-Temperature-Control-Library

#include <Wire.h>
#include <I2CSoilMoistureSensor.h> // https://github.com/Apollon77/I2CSoilMoistureSensor

LogHandler logHandler;
LogSerialModule serialModule(115200);

GenericLog Log    (logHandler);
NamedLog   LogWiFi(logHandler, "WiFi");
NamedLog   LogMqtt(logHandler, "MQTT");
NamedLog   LogDallas(logHandler, "Dallas");
NamedLog   LogChirp(logHandler, "Chirp");

ThreadController threadControl = ThreadController();

WiFiClient espClient;
PubSubClient mqttClient(MQTT_SERVER, 1883, espClient);
PubSubClientTools mqtt(mqttClient);

const String ESP_ID = upperCaseStr(String(ESP.getChipId(), HEX));
const String BOARD_ID = String("PlantBot_")+ESP_ID;
char   BOARD_ID_CHAR[50];
String s = "";

void setup() {
  BOARD_ID.toCharArray(BOARD_ID_CHAR, 50);
  
  Serial.begin(115200);
  logHandler.addModule(&serialModule);
  Log.info("Initializing..");
  Log.info( String("ESP ID: ") + ESP_ID );

  // Init Submodules
  setup_WiFi();
  setup_MQTT();
  setup_ArduinoOTA();

  setup_Maintanance();
  setup_Sensor_Dallas();

  setup_Sensor_Chirp();

  Log.info("Setup done");
}

void loop() {  
  loop_MQTT();
  loop_ArduinoOTA();

  threadControl.run();
}
