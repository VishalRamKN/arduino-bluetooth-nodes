/*
  BT_Hub_All - Arduino UNO + HC-05 controlling all five components
  Components: I2C LCD, HC-SR04 ultrasonic, SG90 servo, analog pulse sensor, LED
  Every command ends with a newline. Send HELP for the list.
  License: MIT (see LICENSE)
*/
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <SoftwareSerial.h>
#include <Servo.h>
#include <string.h>
#include <stdlib.h>

// ---------- Pin map ----------
const uint8_t BT_RX_PIN = 2;   // UNO receives on D2  <- HC-05 TXD
const uint8_t BT_TX_PIN = 3;   // UNO sends on D3     -> HC-05 RXD (via divider)
const uint8_t ECHO_PIN  = 6;   // HC-SR04 Echo
const uint8_t TRIG_PIN  = 7;   // HC-SR04 Trig
const uint8_t SERVO_PIN = 9;   // SG90 orange wire
const uint8_t LED_PIN   = 12;  // LED anode through 220 ohm
const uint8_t PULSE_PIN = A0;  // Pulse sensor S pin

// ---------- Settings ----------
const uint8_t LCD_ADDR = 0x27; // change to 0x3F if the LCD stays blank
const long    BAUD     = 9600; // HC-05 default data-mode speed
const uint8_t CMD_MAX  = 64;   // longest command accepted

SoftwareSerial bt(BT_RX_PIN, BT_TX_PIN);
LiquidCrystal_I2C lcd(LCD_ADDR, 16, 2);
Servo servo;

// ---------- State ----------
int  servoAngle = 90;
bool ledOn      = false;
bool hrAuto     = false;

// One command buffer for Bluetooth, one for the USB Serial Monitor
char    btBuf[CMD_MAX + 1];
uint8_t btLen = 0;
char    usbBuf[CMD_MAX + 1];
uint8_t usbLen = 0;

// ---------- Heartbeat detection ----------
const unsigned long HR_SAMPLE_MS  = 10;   // 100 samples per second
const int           HR_MIN_SWING  = 40;   // smaller swing = no finger or noise
const unsigned long HR_MIN_IBI    = 300;  // ignore beats faster than 200 BPM
const unsigned long HR_MAX_IBI    = 2000; // ignore gaps longer than 30 BPM
const unsigned long HR_TIMEOUT_MS = 3000; // no beat for 3 s = pulse lost

unsigned long hrLastSample = 0;
unsigned long hrLastBeat   = 0;
int           hrPeak = 512, hrTrough = 512;
bool          hrAbove = false;
unsigned long hrIbi[5];
uint8_t       hrIbiCount = 0, hrIbiIndex = 0;
int           bpm = 0;

// ---------- Replies go to the phone and to USB ----------
void reply(const char *msg) {
  bt.println(msg);
  Serial.println(msg);
}
void reply(const __FlashStringHelper *msg) {
  bt.println(msg);
  Serial.println(msg);
}

// ---------- 1. LCD ----------
void showOnLcd(const char *text) {
  lcd.clear();
  lcd.setCursor(0, 0);
  size_t n = strlen(text);
  for (size_t i = 0; i < n && i < 32; i++) {
    if (i == 16) lcd.setCursor(0, 1);   // wrap to line 2
    lcd.print(text[i]);
  }
}

// ---------- 2. Ultrasonic ----------
float readDistanceCm() {
  digitalWrite(TRIG_PIN, LOW);
  delayMicroseconds(2);
  digitalWrite(TRIG_PIN, HIGH);
  delayMicroseconds(10);              // 10 us trigger pulse
  digitalWrite(TRIG_PIN, LOW);
  unsigned long dur = pulseIn(ECHO_PIN, HIGH, 30000UL); // 30 ms timeout
  if (dur == 0) return -1.0;          // no echo = out of range
  return dur * 0.0343 / 2.0;          // speed of sound, there and back
}

// ---------- 3. Servo ----------
bool isNumber(const char *s) {
  if (*s == '\0' || strlen(s) > 3) return false;
  for (; *s; s++) {
    if (*s < '0' || *s > '9') return false;
  }
  return true;
}

void setServo(const char *arg) {
  while (*arg == ' ') arg++;
  if (!isNumber(arg) || atoi(arg) > 180) {
    reply(F("Error: use SERVO:0 to SERVO:180"));
    return;
  }
  servoAngle = atoi(arg);
  servo.write(servoAngle);
  char out[40];
  snprintf(out, sizeof(out), "Servo moved to %d deg", servoAngle);
  reply(out);
}

// ---------- 4. Heartbeat ----------
void updateHeartbeat() {
  unsigned long now = millis();
  if (now - hrLastSample < HR_SAMPLE_MS) return;
  hrLastSample = now;

  int v = analogRead(PULSE_PIN);

  // Envelope: jumps to new highs/lows, then slowly relaxes toward the signal
  hrPeak   = (v > hrPeak)   ? v : hrPeak - 1;
  hrTrough = (v < hrTrough) ? v : hrTrough + 1;
  int swing = hrPeak - hrTrough;
  int mid   = (hrPeak + hrTrough) / 2;
  int band  = swing / 8;              // hysteresis so noise can't double-count

  if (swing < HR_MIN_SWING) {
    hrAbove = false;                  // too flat: no finger on the sensor
  } else if (!hrAbove && v > mid + band) {
    hrAbove = true;                   // rising edge = one beat
    unsigned long ibi = now - hrLastBeat;
    if (ibi >= HR_MIN_IBI) {
      hrLastBeat = now;
      if (ibi <= HR_MAX_IBI) {
        hrIbi[hrIbiIndex] = ibi;
        hrIbiIndex = (hrIbiIndex + 1) % 5;
        if (hrIbiCount < 5) hrIbiCount++;
        unsigned long sum = 0;
        for (uint8_t i = 0; i < hrIbiCount; i++) sum += hrIbi[i];
        bpm = 60000UL / (sum / hrIbiCount);
        if (hrAuto && hrIbiCount >= 2) {
          char out[32];
          snprintf(out, sizeof(out), "Beat! BPM: %d", bpm);
          reply(out);
        }
      }
    }
  } else if (hrAbove && v < mid - band) {
    hrAbove = false;                  // signal fell back: ready for next beat
  }

  if (bpm > 0 && now - hrLastBeat > HR_TIMEOUT_MS) {
    bpm = 0;
    hrIbiCount = 0;
    hrIbiIndex = 0;
    if (hrAuto) reply(F("Pulse lost - rest finger on sensor"));
  }
}

void reportHeartRate() {
  if (bpm == 0) {
    reply(F("No pulse yet - rest finger on sensor for 5 s"));
    return;
  }
  char out[40];
  snprintf(out, sizeof(out), "Heart rate: %d BPM%s", bpm,
           hrIbiCount < 3 ? " (settling)" : "");
  reply(out);
}

// ---------- Status and help ----------
void reportStatus() {
  char out[72];
  snprintf(out, sizeof(out), "Servo: %d deg | LED: %s | HR: %d BPM | Auto HR: %s",
           servoAngle, ledOn ? "ON" : "OFF", bpm, hrAuto ? "ON" : "OFF");
  reply(out);
}

void showHelp() {
  reply(F("Commands:"));
  reply(F("LCD:<text>     show text on LCD"));
  reply(F("LCDCLR         clear LCD"));
  reply(F("DIST           distance in cm"));
  reply(F("SERVO:<0-180>  move servo"));
  reply(F("SERVO?         servo angle"));
  reply(F("HR             heart rate now"));
  reply(F("HRON / HROFF   beat alerts on/off"));
  reply(F("LED:ON / LED:OFF"));
  reply(F("STATUS         everything"));
}

// ---------- Command dispatcher ----------
void handleCommand(char *cmd) {
  while (*cmd == ' ') cmd++;                          // trim front
  size_t n = strlen(cmd);
  while (n > 0 && cmd[n - 1] == ' ') cmd[--n] = '\0'; // trim back
  if (n == 0) return;

  char out[64];

  if (strncasecmp(cmd, "LCD:", 4) == 0) {
    const char *text = cmd + 4;
    while (*text == ' ') text++;
    showOnLcd(text);
    if (strlen(text) > 32) reply(F("LCD: shown (cut to 32 chars)"));
    else reply(F("LCD: shown"));
  } else if (strcasecmp(cmd, "LCDCLR") == 0) {
    lcd.clear();
    reply(F("LCD: cleared"));
  } else if (strcasecmp(cmd, "DIST") == 0) {
    float d = readDistanceCm();
    if (d < 0) {
      reply(F("Distance: out of range"));
    } else {
      char num[10];
      dtostrf(d, 0, 1, num);                          // float -> text, 1 decimal
      snprintf(out, sizeof(out), "Distance: %s cm", num);
      reply(out);
    }
  } else if (strncasecmp(cmd, "SERVO:", 6) == 0) {
    setServo(cmd + 6);
  } else if (strcasecmp(cmd, "SERVO?") == 0) {
    snprintf(out, sizeof(out), "Servo is at %d deg", servoAngle);
    reply(out);
  } else if (strcasecmp(cmd, "LED:ON") == 0) {
    ledOn = true;
    digitalWrite(LED_PIN, HIGH);
    reply(F("LED: ON"));
  } else if (strcasecmp(cmd, "LED:OFF") == 0) {
    ledOn = false;
    digitalWrite(LED_PIN, LOW);
    reply(F("LED: OFF"));
  } else if (strcasecmp(cmd, "HR") == 0) {
    reportHeartRate();
  } else if (strcasecmp(cmd, "HRON") == 0) {
    hrAuto = true;
    reply(F("Auto heartbeat: ON"));
  } else if (strcasecmp(cmd, "HROFF") == 0) {
    hrAuto = false;
    reply(F("Auto heartbeat: OFF"));
  } else if (strcasecmp(cmd, "STATUS") == 0) {
    reportStatus();
  } else if (strcasecmp(cmd, "HELP") == 0) {
    showHelp();
  } else {
    snprintf(out, sizeof(out), "Unknown: %s (send HELP)", cmd);
    reply(out);
  }
}

// ---------- Read a line from any serial port ----------
void pollStream(Stream &s, char *buf, uint8_t &len) {
  while (s.available()) {
    char c = s.read();
    if (c == '\n' || c == '\r') {                     // end of command
      if (len > 0) {
        buf[len] = '\0';
        handleCommand(buf);
        len = 0;
      }
    } else if (len < CMD_MAX) {
      buf[len++] = c;
    }
  }
}

// ---------- Setup and main loop ----------
void setup() {
  Serial.begin(BAUD);
  bt.begin(BAUD);

  pinMode(TRIG_PIN, OUTPUT);
  pinMode(ECHO_PIN, INPUT);
  pinMode(LED_PIN, OUTPUT);
  digitalWrite(LED_PIN, LOW);

  servo.attach(SERVO_PIN);
  servo.write(servoAngle);                          // start at 90 deg

  lcd.init();
  lcd.backlight();
  showOnLcd("BT Hub ready    Send HELP");

  reply(F("BT Hub ready. Send HELP for commands."));
}

void loop() {
  pollStream(bt, btBuf, btLen);       // commands from the phone
  pollStream(Serial, usbBuf, usbLen); // commands from Serial Monitor
  updateHeartbeat();                  // runs all the time, never blocks
}
