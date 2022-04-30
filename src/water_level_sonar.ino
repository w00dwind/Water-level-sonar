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

void checkPhysicalButton()
{

      Blynk.virtualWrite(vPIN_VALVE, digitalRead(VALVE_PIN));
    }



NewPing sonar(SONAR_TRIG, SONAR_ECHO, MAX_DISTANCE); // Настройка пинов и максимального расстояния.



int val;
float val_f;



float dist_3[3] = {0.0, 0.0, 0.0};   // массив для хранения трёх последних измерений
float middle, dist;
// float k;
byte i;
// unsigned long sensTimer;


void check_level()
{
  float val = sonar.ping() / 57.5;



  /// Медианный фильтр + бегущее срднее ///
  if (i > 1) i = 0;
  else i++;

  dist_3[i] = (float)val;                 // получить расстояние в текущую ячейку массива

  dist = middle_of_3(dist_3[0], dist_3[1], dist_3[2]);    // фильтровать медианным фильтром из 3ёх последних измерений

  // delta = abs(dist_filtered - dist);                      // расчёт изменения с предыдущим
  // if (delta > 1) k = 0.7;                                 // если большое - резкий коэффициент
  // else k = 0.1;                                           // если маленькое - плавный коэффициент

  // dist_filtered = dist * k + dist_filtered * (1 - k);     // бегущее среднее после медианного фильтра

  ///
  // Только бегущее среднее

  val_f = val * FILTER_COEF + val_f * (1 - FILTER_COEF);


  int perc = map(val_f, maxLevel, minLevel, 0, 100); // Задать min / max значения измерений датчика и перевести в проценты
  bool valve_state = digitalRead(VALVE_PIN); // Переменная для хранения состояния клапана

  #if (DEBUG == 1)
    Serial.print("water level is : ");
    Serial.print(perc);
    Serial.print('\t');
    Serial.print(dist);
    // Serial.print('\t');
    // Serial.println(dist_filtered);
  #endif
  // send value to blynk
  Blynk.virtualWrite(vPIN_SONAR, perc);
  if (sendTerminal) {
  // printOutput(String(perc) + String("\t") + String(dist) + String("\t") + String(dist_filtered));
  // string_to_output = String(perc) + "\t" + String(val_f) + "\t(" + String(val) + ")";
  printOutput(String(perc) + "\t" + String(val_f) + "\t(" + String(val) + ")\t" + String(valve_state));
                    }
  // int timeMins = (sec % 3600ul) / 60ul;


#if (FILL_AUTO == 1)
if (perc <= 20 && !valve_prev) {
    digitalWrite(VALVE_PIN, 1);
    valve_prev = valve_state;
// #if (DEBUG == 1)
    printOutput(">>> FILL_AUTO_START >> VALVE_STATE - " + String(valve_prev));
// #endif
}
else if (perc >= 100 && valve_prev) {
    digitalWrite(VALVE_PIN, 0);
    valve_prev = 0;
// #if (DEBUG)
    printOutput(">>> FILL_AUTO_STOP >> VALVE_STATE - " + String(valve_prev));
// #endif
    }

#endif
}

void valve_safety()
{
  bool current_state = digitalRead(VALVE_PIN);

  if (current_state && !prev_state_safety)
  {
    valve_timer = millis();
    prev_state_safety = current_state;

    printOutput(">>> Safety timer start ");
  }
  else if ((millis() - valve_timer) >= (valve_timer_thresh * 1000) && prev_state_safety) // && prev_state_safety ?
  {
    digitalWrite(VALVE_PIN, 0);
    prev_state_safety = false;
    printOutput(">>> Valve closed, time: " + String(((millis() - valve_timer) / 1000)) + String(" sec."));
  }

}



void setup() {
  pinMode(VALVE_PIN, OUTPUT);
  Serial.begin(9600);

  WiFi.mode(WIFI_STA); // режим клиента
  WiFi.hostname(esp_hostname); //
  WiFi.begin(ssid, pass);

  while (WiFi.waitForConnectResult() != WL_CONNECTED) {
    Serial.println("Connection Failed! Rebooting...");
    delay(5000);
    ESP.restart();
  }
  ArduinoOTA.onError([](ota_error_t error) {
    ESP.restart();
  });
  ArduinoOTA.setHostname(esp_hostname); // !!!!
  ArduinoOTA.begin();

  Blynk.begin(auth, ssid, pass);
  Blynk.syncVirtual(vPIN_MAX_LEVEL, vPIN_MIN_LEVEL, vPIN_SEND_TERMINAL);
 // timer
 timer.setInterval(1000L, checkPhysicalButton);
 timer.setInterval(5000L, valve_safety);
  timer.setInterval(2000L, check_level);
  timer.setInterval(30000L, sendUptime);
  timerDate = timer.setInterval(5000, []() {
    Blynk.virtualWrite(vPIN_CUR_DATE, getCurrentDate() + String("  ") + getCurrentTime() );
    Blynk.setProperty(vPIN_CUR_DATE, "label", String("WIFI: ") + String(map(WiFi.RSSI(), -105, -40, 0, 100)) + String("% (") + WiFi.RSSI() + String("dB)") + String(" IP: ") + WiFi.localIP().toString());
  });

}

void loop() {
  timer.run();
  Blynk.run();
  ArduinoOTA.handle();
}

// медианный фильтр из 3ёх значений
float middle_of_3(float a, float b, float c) {
  if ((a <= b) && (a <= c)) {
    middle = (b <= c) ? b : c;
  }
  else {
    if ((b <= a) && (b <= c)) {
      middle = (a <= c) ? a : c;
    }
    else {
      middle = (a <= b) ? a : b;
    }
  }
  return middle;
}
