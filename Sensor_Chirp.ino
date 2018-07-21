I2CSoilMoistureSensor* chirp[CHIRP_STOP_ADDR-CHIRP_START_ADDR];

Thread sensorChirpMeassureThread = Thread();

int chirpCount = 0;

void setup_Sensor_Chirp() {
  Wire.begin();
  delay(1000);

  for(int address = CHIRP_START_ADDR; address < CHIRP_STOP_ADDR; address++ ) {
    Wire.beginTransmission(address);

    if (Wire.endTransmission() == 0) {
      I2CSoilMoistureSensor* tmp = new I2CSoilMoistureSensor(address);
      delay(1000);
      LogChirp.info(s+"I2C device found at address 0x"+String(address, HEX)+" FW: "+String(tmp->getVersion(),HEX));
      
      chirp[chirpCount++] = tmp;
    }
  }
  LogChirp.info(s+"Found "+chirpCount+" devices");
  
  sensorChirpMeassureThread.onRun(sensorChirpMeassureFunc);
  sensorChirpMeassureThread.setInterval(CHIRP_INTERVAL);
  threadControl.add(&sensorChirpMeassureThread);
}

void sensorChirpMeassureFunc() {
  for(int i=0; i<chirpCount; i++) {
    I2CSoilMoistureSensor& sensor = *chirp[i];

    // Start measurement, the results will however contain old data
    sensor.getCapacitance();
    sensor.getTemperature();
    sensor.startMeasureLight();

    // Wait until measurement is done
    int count = 0;
    while (sensor.isBusy()) {
      delay(10);
      count++;
      if (count >= 10) {
        return;
      }
    }

    // Get recent data & publish
    String output;
    DynamicJsonBuffer jsonBuffer;
  
    JsonObject& root = jsonBuffer.createObject();
    root["val"] = sensor.getCapacitance();
    root["temperature"] = sensor.getTemperature()/10.0;
    root["brightness"] = sensor.getLight();
  
    root.printTo(output);
    mqtt.publish(s+MQTT_PREFIX+"/status/"+BOARD_ID+"/sensor/"+sensor.getAddress(), output, true);
  }
}

