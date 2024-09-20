#include <Adafruit_GFX.h>
#include <Wire.h>
#include <Adafruit_SSD1306.h>
#include <DHTesp.h>
#include <WiFi.h>
#include <PubSubClient.h>
#include <NTPClient.h>
#include <WiFiUdp.h>
#include <ESP32Servo.h>

WiFiClient espClient;
PubSubClient mqttClient(espClient);

char LDR_ar[6];
char ANG_ar[6];
char whichLDR[6];

static float val_LDR1 = 0.0;
static float val_LDR2 = 0.0;
static float val_LDR = 0.0;
const float RL = 50;
const float GAMMA = 0.7;
String side;
const int servo_pin = 18;
float Dfactor;
float gamma1 = 0.75;
float offset = 30;

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET -1
#define SCREEN_ADDRESS 0x3C
#define BUZZER 5
#define LED_1 15
#define PB_cancel 34
#define PB_OK 32
#define PB_UP 33
#define PB_DOWN 35
#define DHTPIN 12
#define LIGHT_SENSOR_PIN1 39
#define LIGHT_SENSOR_PIN2 36

int angle = 0;

#define NTP_SERVER     "pool.ntp.org"
#define UTC_OFFSET     1
#define UTC_OFFSET_DST 0

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);
DHTesp dhtSensor;
Servo servo_motor;

int days = 0;
int hours = 0;
int minutes = 0;
int seconds = 0;
String months;

bool alarm_enabled = true;
int n_alarms = 3;
int alarm_hours[] = {0, 1, 18};
int alarm_minutes[] = {1, 10, 31}; //will play the alrm for the whole minute
bool alarm_triggered[] = {false, false, false};

int C = 262;
int D = 294;
int E = 330;
int F = 349;
int G = 392;
int A = 440;
int B = 194;
int C_H = 523;
int notes[] = {C, D, E, F, G, A, B, C_H};
int n_notes = 8;

int current_mode = 0;
int max_modes = 5;
String modes[] = {"Set Time\nZone", "Set Alarm\nOne", "Set Alarm\nTwo", "Set Alarm\nThree", "Disable\nAll Alarms"};
String modes2[] = {"+", "-"};

String extext;
String textdays;
String texthours;
String textminutes;
String textseconds;

int minutesoffset = 30;
int hoursoffset = 5;
int signoffset;

String currenttemp;
char tempAr[6];
String currenthumi;

void setup() {
  SetupMqtt();

  pinMode(BUZZER, OUTPUT);
  pinMode(LED_1, OUTPUT);
  pinMode(PB_cancel, INPUT);
  pinMode(PB_OK, INPUT);
  pinMode(PB_UP, INPUT);
  pinMode(PB_DOWN, INPUT);
  ledcSetup(0, 5000, 8); // LEDC channel 0, 5 kHz frequency, 8-bit resolution
  ledcAttachPin(BUZZER, 0);

  dhtSensor.setup(DHTPIN, DHTesp::DHT22);

  Serial.begin(115200);
  Serial.print("Number: ");
  if (!display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS)) {
    Serial.println(F("SSD1306 allocation failed"));
    for (;;);
  }
  display.display();
  delay(500);

  WiFi.begin("Wokwi-GUEST", "", 6);
  while (WiFi.status() != WL_CONNECTED) {
    delay(250);
    display.clearDisplay();
    print_line("Connecting\nto WIFI", 0, 0, 2);
  }

  display.clearDisplay();
  print_line("Connected\nto WIFI", 0, 0, 2);
  delay(500);

  configTime(UTC_OFFSET, UTC_OFFSET_DST, NTP_SERVER);


  display.clearDisplay();
  print_line("Welcome\nto the\nMedi Box!", 0, 0, 2);
  delay(2500);
  display.clearDisplay();
  servo_motor.attach(servo_pin, 500, 2400);
}

void loop() {
  if (!mqttClient.connected()) {
    connectToBroker();
  }
  float analogValue = light_intensity();
  // Serial.print("Analog Value = ");
  // Serial.println(analogRead(LIGHT_SENSOR_PIN));

  mqttClient.loop();
  String(analogValue, 2).toCharArray(LDR_ar, 6);
  mqttClient.publish("PRABATH-LDR", LDR_ar);
  side.toCharArray(whichLDR, 6);
  mqttClient.publish("PRABATH-whichLDR", whichLDR);

  String angleStr = String(angle);
  angleStr.toCharArray(ANG_ar, 6);

  mqttClient.publish("PRABATH-ANG", ANG_ar);

  angle = calculate_angle(offset, Dfactor, analogValue, gamma1);
  Serial.print("Minimus Angle: "); Serial.print(offset);
  Serial.print("Dfactor: "); Serial.print(Dfactor);
  Serial.print("Light Intensity: "); Serial.print(analogValue);
  Serial.print("Controlling Factor: "); Serial.print(gamma1);
  Serial.print("Motor Angle: "); Serial.println(angle);
  servo_motor.write(angle);


  check_temp();
  update_time_with_check_alarms();
  if (digitalRead(PB_OK) == LOW) {
    delay(200);
    go_to_menu();
  }
}





void connectToBroker() {
  while (!mqttClient.connected()) {
    Serial.print("Attempting MQTT connection...");
    if (mqttClient.connect("ESP32-345464")) {
      Serial.print("connected");
      mqttClient.subscribe("PRABATH-Command1");
      mqttClient.subscribe("PRABATH-Command2");
      mqttClient.subscribe("min_angle");
      mqttClient.subscribe("cont_fac");
    } else {
      Serial.print("failed");
      Serial.print(mqttClient.state());
      delay(5000);
    }
  }
}

void SetupMqtt() {
  mqttClient.setServer("test.mosquitto.org", 1883);
  mqttClient.setCallback(receiveCallback);
}

void receiveCallback(char* topic, byte* payload, unsigned int length) {
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("] ");

  char payloadCharAr[length];
  for (int i = 0; i < length; i++) {
    Serial.print((char)payload[i]);
    payloadCharAr[i] = (char)payload[i];
  }
  Serial.println("] ");

  if (strcmp(topic, "PRABATH-Command1") == 0) {
    gamma1 = atof(payloadCharAr);
    Serial.print("Controlling Factor :");
    Serial.println(gamma1);
  }
  if (strcmp(topic, "PRABATH-Command2") == 0) {
    offset = atof(payloadCharAr);
    Serial.print("Minimum Angle :");
    Serial.println(offset);
  }


}

float light_intensity() {
  val_LDR1 = analogRead(LIGHT_SENSOR_PIN1);
  val_LDR2 = analogRead(LIGHT_SENSOR_PIN2);

  if (val_LDR1 < val_LDR2) {
    val_LDR = val_LDR1;
    side = "Right";
    Dfactor = 0.5;
  }
  else {
    val_LDR = val_LDR2;
    side = "Left";
    Dfactor = 1.5;
  }

  val_LDR = map(val_LDR, 4095, 0, 1024, 0);
  float vol = val_LDR / 1024. * 5;
  float res = 2000 * vol / (1 - vol / 5);
  float lux = pow(RL * 1e3 * pow(10, GAMMA) / res, (1 / GAMMA)) / 85168.02;
  return lux;
}

float calculate_angle(float theta_offset, float D, float lightIntensity, float gamma) {
  float angle1 = theta_offset * D + (180 - theta_offset) * lightIntensity * gamma;
  float angle2 = 180;
  float theta = fmin(angle1, angle2); // Calculate the minimum of the two angles
  return theta;
}





void print_line(String text, int column, int row, int text_size) {
  display.setTextSize(text_size);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(column, row);
  display.println(text);
  display.display();
}

void print_2lines(String text, int column, int row, int text_size, String text2, int column2, int row2, int text_size2) {
  display.setTextSize(text_size);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(column, row);
  display.println(text);
  display.setTextSize(text_size2);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(column2, row2);
  display.println(text2);
  display.display();
}

void print_time_now(void) {
  display.clearDisplay();
  if (days < 10) {
    textdays = "0" + String(days);
  } else {
    textdays = String(days);
  }
  if (hours < 10) {
    texthours = "0" + String(hours);
  } else {
    texthours = String(hours);
  }
  if (minutes < 10) {
    textminutes = "0" + String(minutes);
  } else {
    textminutes = String(minutes);
  }
  if (seconds < 10) {
    textseconds = "0" + String(seconds);
  } else {
    textseconds = String(seconds);
  }
  extext = months + " " + textdays + "\n" + texthours + ":" + textminutes + ":" + textseconds;
  print_2lines(extext, 0, 0, 2, currenttemp + "\n" + currenthumi, 0, 40, 1);
}



void update_time() {
  struct tm timeinfo;
  if (signoffset = 0) {
    setTimezone("UTC+" + String(hoursoffset) + ":" + String(minutesoffset));
  } else {
    setTimezone("UTC-" + String(hoursoffset) + ":" + String(minutesoffset));

  }
  getLocalTime(&timeinfo);
  char timeMonth[7];
  strftime(timeMonth, 7, "%B", &timeinfo);
  months = timeMonth;
  char timeHour[3];
  strftime(timeHour, 3, "%H", &timeinfo);
  hours = atoi(timeHour);
  char timeMinute[3];
  strftime(timeMinute, 3, "%M", &timeinfo);
  minutes = atoi(timeMinute);
  char timeSecond[3];
  strftime(timeSecond, 3, "%S", &timeinfo);
  seconds = atoi(timeSecond);
  char timeDay[3];
  strftime(timeDay, 3, "%d", &timeinfo);
  days = atoi(timeDay);
}

void ring_alarm() {
  display.clearDisplay();
  print_line("TIME FOR \n MEDICINE", 0, 0, 2);
  digitalWrite(LED_1, HIGH);
  bool break_happened = false;
  while (break_happened == false && digitalRead(PB_cancel) == HIGH) {
    for (int i = 0; i < n_notes; i++) {
      if (digitalRead(PB_cancel) == LOW) {
        delay(200);
        break_happened = true;
        break;
      }
      tone(BUZZER, notes[i]);
      delay(500);
      noTone(BUZZER);
      delay(2);
    }
  }
  digitalWrite(LED_1, LOW);
  display.clearDisplay();
}


void update_time_with_check_alarms() {
  update_time();
  print_time_now();
  if (alarm_enabled == true) {
    for (int i = 0; i < n_alarms; i++) {
      if (alarm_triggered[i] == false && alarm_hours[i] == hours && alarm_minutes[i] == minutes) {
        ring_alarm();
        alarm_triggered[i] = true;
      }
    }
  }
}

int wait_for_button_press() {
  while (true) {
    if (digitalRead(PB_UP) == LOW) {
      delay(200);
      return PB_UP;
    }
    else if (digitalRead(PB_DOWN) == LOW) {
      delay(200);
      return PB_DOWN;
    }
    else if (digitalRead(PB_OK) == LOW) {
      delay(200);
      return PB_OK;
    }
    else if (digitalRead(PB_cancel) == LOW) {
      delay(200);
      return PB_cancel;
    }
    update_time();
  }
}

void go_to_menu() {
  while (digitalRead(PB_cancel) == HIGH) {
    display.clearDisplay();
    print_2lines(modes[current_mode], 0, 0, 2, String(current_mode + 1), 112, 50, 1);
    int pressed = wait_for_button_press();
    if (pressed == PB_UP) {
      delay(200);
      current_mode += 1;
      current_mode = current_mode % max_modes;
    }
    else if (pressed == PB_DOWN) {
      delay(200);
      current_mode -= 1;
      if (current_mode < 0) {
        current_mode = max_modes - 1;
      }
    }
    else if (pressed == PB_OK) {
      delay(200);
      Serial.println(current_mode);
      run_mode(current_mode);
    }
    else if (pressed == PB_cancel) {
      delay(200);
      break;
    }
  }
}

void set_time() {
  int temp_sign = signoffset;
  while (true) {
    display.clearDisplay();
    print_2lines("Sign\nOffset:", 0, 0, 2, String(modes2[temp_sign]), 50, 45, 2);
    int pressed = wait_for_button_press();
    if (pressed == PB_UP) {
      delay(200);
      temp_sign += 1;
      if (temp_sign > 1) {
        temp_sign = 0;
      }
    }
    else if (pressed == PB_DOWN) {
      delay(200);
      temp_sign -= 1;
      if (temp_sign < 0) {
        temp_sign = 1;
      }
    }
    else if (pressed == PB_OK) {
      delay(200);
      signoffset = temp_sign;
      break;
    }
    else if (pressed == PB_cancel) {
      delay(200);
      break;
    }
  }
  int temp_hour = hoursoffset;
  while (true) {
    display.clearDisplay();
    print_2lines("Hours\nOffset:", 0, 0, 2, String(temp_hour), 50, 45, 2);
    int pressed = wait_for_button_press();
    if (pressed == PB_UP) {
      delay(200);
      temp_hour += 1;
      if (temp_hour > 12) {
        temp_hour = -12;
      }
    }
    else if (pressed == PB_DOWN) {
      delay(200);
      temp_hour -= 1;
      if (temp_hour < -12) {
        temp_hour = 12;
      }
    }
    else if (pressed == PB_OK) {
      delay(200);
      hoursoffset = temp_hour;
      break;
    }
    else if (pressed == PB_cancel) {
      delay(200);
      break;
    }
  }
  int temp_minutes = minutesoffset;
  while (true) {
    display.clearDisplay();
    print_2lines("Minutes\nOffset:", 0, 0, 2, String(temp_minutes), 50, 45, 2);
    int pressed = wait_for_button_press();
    if (pressed == PB_UP) {
      delay(200);
      temp_minutes += 30;
      temp_minutes = temp_minutes % 60;
    }
    else if (pressed == PB_DOWN) {
      delay(200);
      temp_minutes -= 30;
      if (temp_minutes < 0) {
        temp_minutes = 30;
      }
    }
    else if (pressed == PB_OK) {
      delay(200);
      minutesoffset = temp_minutes;
      break;
    }
    else if (pressed == PB_cancel) {
      delay(200);
      break;
    }
  }
  display.clearDisplay();
  print_line("Time Zone\nChanged!", 0, 0, 2);
  delay(1000);
}

void set_alarm(int alarm) {
  int temp_hour = alarm_hours[alarm];
  while (true) {
    display.clearDisplay();
    print_2lines("Enter\nHour:", 0, 0, 2, String(temp_hour), 50, 45, 2);
    int pressed = wait_for_button_press();
    if (pressed == PB_UP) {
      delay(200);
      temp_hour += 1;
      temp_hour = temp_hour % 24;
    }
    else if (pressed == PB_DOWN) {
      delay(200);
      temp_hour -= 1;
      if (temp_hour < 0) {
        temp_hour = 23;
      }
    }
    else if (pressed == PB_OK) {
      delay(200);
      alarm_hours[alarm] = temp_hour;
      break;
    }
    else if (pressed == PB_cancel) {
      delay(200);
      break;
    }
  }
  int temp_minutes = alarm_minutes[alarm];
  while (true) {
    display.clearDisplay();
    print_2lines("Enter\nMinute:", 0, 0, 2, String(temp_minutes), 50, 45, 2);
    int pressed = wait_for_button_press();
    if (pressed == PB_UP) {
      delay(200);
      temp_minutes += 1;
      temp_minutes = temp_minutes % 60;
    }
    else if (pressed == PB_DOWN) {
      delay(200);
      temp_minutes -= 1;
      if (temp_minutes < 0) {
        temp_minutes = 59;
      }
    }
    else if (pressed == PB_OK) {
      delay(200);
      alarm_minutes[alarm] = temp_minutes;
      break;
    }
    else if (pressed == PB_cancel) {
      delay(200);
      break;
    }
  }
  display.clearDisplay();
  print_line("Alarm is\nset!", 0, 0, 2);
  delay(1000);
}

void run_mode(int mode) {
  if (mode == 0) {
    set_time();
  }
  else if (mode == 1 || mode == 2 || mode == 3) {
    set_alarm(mode - 1);
  }
  else if (mode == 4) {
    alarm_enabled = false;
    display.clearDisplay();
    print_line("All Alarms\nDisabled!", 0, 0, 2);
    delay(1000);
  }
}

void check_temp() {
  TempAndHumidity data = dhtSensor.getTempAndHumidity();
  currenttemp = "TEMP GOOD";

  if (data.temperature >= 32) {
    currenttemp = "TEMP HIGH";
    tone(BUZZER, 440);
  }
  else if (data.temperature <= 26) {
    currenttemp = "TEMP LOW";
    tone(BUZZER, 440);
  }
  currenthumi = "HUMIDITY GOOD";
  if (data.humidity >= 80) {
    currenthumi = "HUMIDITY HIGH";
    tone(BUZZER, 440);
  }
  else if (data.humidity <= 60) {
    currenthumi = "HUMIDITY LOW";
    tone(BUZZER, 440);
  }
  if (currenthumi == "HUMIDITY GOOD" && currenttemp == "TEMP GOOD") {
    noTone(BUZZER);
  }
}

void setTimezone(String timezone) {
  setenv("TZ", timezone.c_str(), 1);
  tzset();
}

