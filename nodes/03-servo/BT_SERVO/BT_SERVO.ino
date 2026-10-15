/* Node 3 - BT-SERVO : Arduino UNO + HC-05 + SG90 servo
   License: MIT (see LICENSE) */
#include <SoftwareSerial.h>
#include <Servo.h>
#include <string.h>
#include <stdlib.h>

const char NODE_NAME[] = "BT-SERVO";
const uint8_t SERVO_PIN = 9;

SoftwareSerial bt(2, 3);            // D2 <- HC-05 TXD, D3 -> HC-05 RXD (divider)
Servo servo;

const uint8_t CMD_MAX = 64;
char btBuf[CMD_MAX + 1];  uint8_t btLen = 0;
char usbBuf[CMD_MAX + 1]; uint8_t usbLen = 0;

int servoAngle = 90;                // start position

void reply(const char *m)                { bt.println(m); Serial.println(m); }
void reply(const __FlashStringHelper *m) { bt.println(m); Serial.println(m); }

char *trim(char *s) {
  while (*s == ' ') s++;
  size_t n = strlen(s);
  while (n > 0 && s[n - 1] == ' ') s[--n] = '\0';
  return s;
}

// 1 to 3 digits only
bool isNumber(const char *s) {
  if (*s == '\0' || strlen(s) > 3) return false;
  for (; *s; s++) if (*s < '0' || *s > '9') return false;
  return true;
}

void moveServo(const char *arg) {
  if (!isNumber(arg) || atoi(arg) > 180) {
    reply(F("Error: send a number 0 to 180"));
    return;
  }
  servoAngle = atoi(arg);
  servo.write(servoAngle);
  char out[32];
  snprintf(out, sizeof(out), "Servo moved to %d deg", servoAngle);
  reply(out);
}

void handleCommand(char *cmd) {
  cmd = trim(cmd);
  if (*cmd == '\0') return;
  char out[48];

  if (*cmd >= '0' && *cmd <= '9') {
    moveServo(cmd);                   // bare number, e.g. "90"
  } else if (strncasecmp(cmd, "SERVO:", 6) == 0) {
    moveServo(trim(cmd + 6));
  } else if (strcasecmp(cmd, "STATUS") == 0 || strcasecmp(cmd, "SERVO?") == 0) {
    snprintf(out, sizeof(out), "Servo is at %d deg", servoAngle);
    reply(out);
  } else if (strcasecmp(cmd, "PING") == 0) {
    snprintf(out, sizeof(out), "%s here", NODE_NAME);
    reply(out);
  } else if (strcasecmp(cmd, "HELP") == 0) {
    reply(F("BT-SERVO commands:"));
    reply(F("0..180        move to angle"));
    reply(F("SERVO:<0-180> same, long form"));
    reply(F("STATUS        current angle"));
    reply(F("PING          node name"));
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
  servo.attach(SERVO_PIN);
  servo.write(servoAngle);            // go to 90 deg on power-up
  reply(F("BT-SERVO ready at 90 deg. Send HELP."));
}

void loop() {
  pollStream(bt, btBuf, btLen);
  pollStream(Serial, usbBuf, usbLen);
}
