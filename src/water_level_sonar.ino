#define BLYNK_PRINT Serial

#include <NewPing.h>
#include "string.h"
#include <Wire.h>
#include <SimpleTimer.h>
#include <ESP8266WiFi.h>
#include <WiFiUdp.h>
#include <ArduinoOTA.h>
#include <BlynkSimpleEsp8266.h>
#include <TimeLib.h>
#include <WidgetRTC.h>
#include "uptime.h"
#include "secret.h"
#include "settings.h"
#include "functions.h"


NewPing sonar(SONAR_TRIG, SONAR_ECHO, MAX_DISTANCE); // Настройка пинов и максимального расстояния.



BLYNK_WRITE(vPIN_MIN_LEVEL)
{
  minLevel = param.asInt();
}

BLYNK_WRITE(vPIN_MAX_LEVEL)
{
  maxLevel = param.asInt();
}
BLYNK_WRITE(vPIN_SEND_TERMINAL)
{
  sendTerminal = param.asInt();
}

BLYNK_WRITE(vPIN_VALVE) {
  virtual_valve_state = param.asInt();
  digitalWrite(VALVE_PIN, virtual_valve_state);
}



void check_level()
{
  float val = sonar.ping() / 57.5;


  // Фильтр
  val_f = val * FILTER_COEF + val_f * (1 - FILTER_COEF);


  int perc = map(val_f, maxLevel, minLevel, 0, 100); // Задать min / max значения измерений датчика и перевести в проценты
  bool valve_state = digitalRead(VALVE_PIN); // Переменная для хранения состояния клапана

  #if (DEBUG == 1)
    Serial.print("water level is : ");
    Serial.print(perc);
    Serial.print('\t');
    Serial.print(dist);
  #endif
  // send value to blynk
  Blynk.virtualWrite(vPIN_SONAR, perc);
  if (sendTerminal) {
  printOutput(String(perc) + "\t" + String(val_f) + "\t(" + String(val) + ")\t" + String(valve_state));
                    }
  // int timeMins = (sec % 3600ul) / 60ul;


#if (FILL_AUTO == 1)
if (perc <= 20 && !valve_prev) {
    digitalWrite(VALVE_PIN, 1);
    valve_prev = true;
    printOutput(">>> FILL_AUTO_START >> VALVE_STATE - " + String(valve_state));

}
else if (perc >= 100 && valve_prev) {
    digitalWrite(VALVE_PIN, 0);
    valve_prev = false;
    printOutput(">>> FILL_AUTO_STOP >> VALVE_STATE - " + String(valve_state));

    }

#endif
}

void valve_safety()
{
  bool current_state = digitalRead(VALVE_PIN);

  if (current_state && !prev_state_safety)
  {
    // valve_timer = millis();
    prev_state_safety = true;

    printOutput(">>> Safety timer start ");
  }
  else if (prev_state_safety && current_state
) //
  {
    digitalWrite(VALVE_PIN, 0);
    prev_state_safety = false;
    printOutput(">>> Valve closed, by safety timer.");
  }
  else {
     Blynk.virtualWrite(D7, digitalRead(VALVE_PIN));
  }

}



void setup() {
  pinMode(VALVE_PIN, OUTPUT);
  ArduinoOTA.onError([](ota_error_t error) {
    ESP.restart();
  });
  ArduinoOTA.setHostname(esp_hostname); // !!!!
  ArduinoOTA.begin();
  Serial.begin(9600);
  Blynk.begin(auth, ssid, pass, SERVER, 8080);
  Blynk.syncVirtual(vPIN_MAX_LEVEL, vPIN_MIN_LEVEL, vPIN_SEND_TERMINAL);



 // timer
 // timer.setInterval(1000L, checkPhysicalButton);
 timer.setInterval(valve_timer_thresh, valve_safety);
  timer.setInterval(2000L, check_level);
  timer.setInterval(30000L, sendUptime);
  timerDate = timer.setInterval(5000, []() {
    Blynk.virtualWrite(vPIN_CUR_DATE, getCurrentDate() + String("  ") + getCurrentTime() );
    Blynk.setProperty(vPIN_CUR_DATE, "label", String("WIFI: ") + String(map(WiFi.RSSI(), -105, -40, 0, 100)) + String("% (") + WiFi.RSSI() + String("dB)") + String(" IP: ") + WiFi.localIP().toString());
  });

}

void loop() {
  ArduinoOTA.handle();
  timer.run();
  Blynk.run();

}
