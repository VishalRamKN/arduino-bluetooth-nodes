/* Node 5 - BT-LED : Arduino UNO + HC-05 + LED
   License: MIT (see LICENSE) */
#include <SoftwareSerial.h>
#include <string.h>

const char NODE_NAME[] = "BT-LED";
const uint8_t LED_PIN = 12;         // anode through 220 ohm

SoftwareSerial bt(2, 3);            // D2 <- HC-05 TXD, D3 -> HC-05 RXD (divider)

const uint8_t CMD_MAX = 64;
char btBuf[CMD_MAX + 1];  uint8_t btLen = 0;
char usbBuf[CMD_MAX + 1]; uint8_t usbLen = 0;

bool ledOn = false;

void reply(const char *m)                { bt.println(m); Serial.println(m); }
void reply(const __FlashStringHelper *m) { bt.println(m); Serial.println(m); }

char *trim(char *s) {
  while (*s == ' ') s++;
  size_t n = strlen(s);
  while (n > 0 && s[n - 1] == ' ') s[--n] = '\0';
  return s;
}

void setLed(bool on) {
  ledOn = on;
  digitalWrite(LED_PIN, on ? HIGH : LOW);
  if (on) reply(F("LED: ON"));
  else    reply(F("LED: OFF"));
}

void handleCommand(char *cmd) {
  cmd = trim(cmd);
  if (*cmd == '\0') return;
  char out[48];

  if (strcasecmp(cmd, "ON") == 0 || strcasecmp(cmd, "LED:ON") == 0 || strcmp(cmd, "1") == 0) {
    setLed(true);
  } else if (strcasecmp(cmd, "OFF") == 0 || strcasecmp(cmd, "LED:OFF") == 0 || strcmp(cmd, "0") == 0) {
    setLed(false);
  } else if (strcasecmp(cmd, "STATUS") == 0) {
    if (ledOn) reply(F("LED is ON"));
    else       reply(F("LED is OFF"));
  } else if (strcasecmp(cmd, "PING") == 0) {
    snprintf(out, sizeof(out), "%s here", NODE_NAME);
    reply(out);
  } else if (strcasecmp(cmd, "HELP") == 0) {
    reply(F("BT-LED commands:"));
    reply(F("ON  / LED:ON  / 1   LED on"));
    reply(F("OFF / LED:OFF / 0   LED off"));
    reply(F("STATUS              LED state"));
    reply(F("PING                node name"));
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
  pinMode(LED_PIN, OUTPUT);
  digitalWrite(LED_PIN, LOW);         // start off
  reply(F("BT-LED ready. Send HELP."));
}

void loop() {
  pollStream(bt, btBuf, btLen);
  pollStream(Serial, usbBuf, usbLen);
}
