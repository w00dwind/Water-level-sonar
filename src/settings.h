BlynkTimer timer;
int timerDate;
WidgetRTC rtc;

// Фильтр бегущее среднее
// #define FILTER_STEP 10000
#define FILTER_COEF 0.5
#define DEBUG 0
// Tank auto filling
#define FILL_AUTO 1

int val; // raw data from Sonar
float val_f; // filtered data from Sonar
float dist_3[3] = {0.0, 0.0, 0.0};   // массив для хранения трёх последних измерений
float middle, dist;
byte i;

// threshold for safety power off valve in seconds
unsigned long valve_timer_thresh = 60;
unsigned long valve_timer;
bool valve_prev = false; // default value to boot


// сонар
#define SONAR_ECHO D6
#define SONAR_TRIG D5
// set pin to manage valve
#define VALVE_PIN  D7



int minLevel;
int maxLevel;
bool sendTerminal;
bool virtual_valve_state;
bool prev_state_safety = false;


// Sonar settings
#define MAX_DISTANCE 200  // Максимальная дистанция измерения
// Замеренные значения на VBM - min 17cm, max 3cm
#define vPIN_VALVE                      V0
#define vPIN_MIN_LEVEL                  V6
#define vPIN_MAX_LEVEL                  V7
#define vPIN_SONAR                      V1
#define vPIN_CUR_DATE                   V8
#define vPIN_TERMINAL                   V20
#define vPIN_UPTIME                     V5
#define vPIN_SEND_TERMINAL              V21

WidgetTerminal terminal(vPIN_TERMINAL);

// #define MIN_LEVEL_CM 17 // Минимальный уровень воды в сантиметрах(чем меньше воды в баке, тем больше расстояние, измеряемое датчиком
// #define MAX_LEVEL_CM 3 // Максимальный уровень воды в сантиметрах
