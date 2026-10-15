/* Node 2 - BT-DIST : Arduino UNO + HC-05 + HC-SR04
   License: MIT (see LICENSE) */
#include <SoftwareSerial.h>
#include <string.h>
#include <stdlib.h>

const char NODE_NAME[] = "BT-DIST";
const uint8_t TRIG_PIN = 7;
const uint8_t ECHO_PIN = 6;

SoftwareSerial bt(2, 3);            // D2 <- HC-05 TXD, D3 -> HC-05 RXD (divider)

const uint8_t CMD_MAX = 64;
char btBuf[CMD_MAX + 1];  uint8_t btLen = 0;
char usbBuf[CMD_MAX + 1]; uint8_t usbLen = 0;

float lastCm = -1;                  // -1 = no reading yet / out of range
unsigned long lastAt = 0;

void reply(const char *m)                { bt.println(m); Serial.println(m); }
void reply(const __FlashStringHelper *m) { bt.println(m); Serial.println(m); }

char *trim(char *s) {
  while (*s == ' ') s++;
  size_t n = strlen(s);
  while (n > 0 && s[n - 1] == ' ') s[--n] = '\0';
  return s;
}

// ---- one raw reading ----
float pingOnce() {
  digitalWrite(TRIG_PIN, LOW);
  delayMicroseconds(2);
  digitalWrite(TRIG_PIN, HIGH);
  delayMicroseconds(10);              // 10 us trigger pulse
  digitalWrite(TRIG_PIN, LOW);
  unsigned long dur = pulseIn(ECHO_PIN, HIGH, 30000UL); // 30 ms = ~5 m
  if (dur == 0) return -1;
  return dur * 0.0343 / 2.0;          // cm, there and back
}

// ---- median of three readings ----
float readDistanceCm() {
  float a = pingOnce(); delay(60);    // let echoes die out between pings
  float b = pingOnce(); delay(60);
  float c = pingOnce();
  if (a > b) { float t = a; a = b; b = t; }
  if (b > c) { float t = b; b = c; c = t; }
  if (a > b) { float t = a; a = b; b = t; }
  return b;                           // middle value
}

void handleCommand(char *cmd) {
  cmd = trim(cmd);
  if (*cmd == '\0') return;
  char out[48], num[10];

  if (strcasecmp(cmd, "DIST") == 0 || strcasecmp(cmd, "D") == 0) {
    lastCm = readDistanceCm();
    lastAt = millis();
    if (lastCm < 0) reply(F("Distance: out of range"));
    else {
      dtostrf(lastCm, 0, 1, num);
      snprintf(out, sizeof(out), "Distance: %s cm", num);
      reply(out);
    }
  } else if (strcasecmp(cmd, "STATUS") == 0) {
    if (lastAt == 0) reply(F("No reading yet. Send DIST."));
    else {
      unsigned long ago = (millis() - lastAt) / 1000;
      if (lastCm < 0) snprintf(out, sizeof(out), "Last: out of range, %lu s ago", ago);
      else { dtostrf(lastCm, 0, 1, num);
             snprintf(out, sizeof(out), "Last: %s cm, %lu s ago", num, ago); }
      reply(out);
    }
  } else if (strcasecmp(cmd, "PING") == 0) {
    snprintf(out, sizeof(out), "%s here", NODE_NAME);
    reply(out);
  } else if (strcasecmp(cmd, "HELP") == 0) {
    reply(F("BT-DIST commands:"));
    reply(F("DIST or D   measure distance"));
    reply(F("STATUS      last reading"));
    reply(F("PING        node name"));
  } else {
    snprintf(out, sizeof(out), "Unknown: %s (send HELP)", cmd);
    reply(out);
  }
}

void pollStream(Stream &s, char *buf, uint8_t &len) {
  while (s.available()) {
    char c = s.read();
    if (c == '\n' || c == '\r') {
      if (len > 0) { buf[len] = '\0'; handleCommand(buf); len = 0; }
    } else if (len < CMD_MAX) {
      buf[len++] = c;
    }
  }
}

void setup() {
  Serial.begin(9600);
  bt.begin(9600);
  pinMode(TRIG_PIN, OUTPUT);
  pinMode(ECHO_PIN, INPUT);
  reply(F("BT-DIST ready. Send HELP."));
}

void loop() {
  pollStream(bt, btBuf, btLen);
  pollStream(Serial, usbBuf, usbLen);
}
