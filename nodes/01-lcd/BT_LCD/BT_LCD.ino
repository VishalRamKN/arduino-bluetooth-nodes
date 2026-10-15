/* Node 1 - BT-LCD : Arduino UNO + HC-05 + I2C LCD 16x2
   License: MIT (see LICENSE) */
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <SoftwareSerial.h>
#include <string.h>

const char NODE_NAME[] = "BT-LCD";
const uint8_t LCD_ADDR = 0x27;      // try 0x3F if the screen stays blank

SoftwareSerial bt(2, 3);            // D2 <- HC-05 TXD, D3 -> HC-05 RXD (divider)
LiquidCrystal_I2C lcd(LCD_ADDR, 16, 2);

const uint8_t CMD_MAX = 64;
char btBuf[CMD_MAX + 1];  uint8_t btLen = 0;
char usbBuf[CMD_MAX + 1]; uint8_t usbLen = 0;
char shown[33] = "";                // what is on the screen now

// ---- replies go to phone and USB ----
void reply(const char *m)                { bt.println(m); Serial.println(m); }
void reply(const __FlashStringHelper *m) { bt.println(m); Serial.println(m); }

char *trim(char *s) {
  while (*s == ' ') s++;
  size_t n = strlen(s);
  while (n > 0 && s[n - 1] == ' ') s[--n] = '\0';
  return s;
}

// ---- LCD ----
void showOnLcd(const char *text) {
  lcd.clear();
  lcd.setCursor(0, 0);
  size_t n = strlen(text);
  for (size_t i = 0; i < n && i < 32; i++) {
    if (i == 16) lcd.setCursor(0, 1);   // wrap to line 2
    lcd.print(text[i]);
  }
  strncpy(shown, text, 32);
  shown[32] = '\0';
}

void printText(const char *text) {
  showOnLcd(text);
  if (strlen(text) > 32) reply(F("LCD: shown (cut to 32 chars)"));
  else reply(F("LCD: shown"));
}

// ---- commands ----
void handleCommand(char *cmd) {
  cmd = trim(cmd);
  if (*cmd == '\0') return;
  char out[48];

  if (strcasecmp(cmd, "PING") == 0) {
    snprintf(out, sizeof(out), "%s here", NODE_NAME);
    reply(out);
  } else if (strcasecmp(cmd, "CLR") == 0) {
    lcd.clear();
    shown[0] = '\0';
    reply(F("LCD: cleared"));
  } else if (strcasecmp(cmd, "STATUS") == 0) {
    if (shown[0] == '\0') reply(F("LCD is blank"));
    else { snprintf(out, sizeof(out), "LCD shows: %s", shown); reply(out); }
  } else if (strcasecmp(cmd, "HELP") == 0) {
    reply(F("BT-LCD commands:"));
    reply(F("<any text>   show on LCD"));
    reply(F("LCD:<text>   show text, even a command word"));
    reply(F("CLR          clear screen"));
    reply(F("STATUS       what is shown"));
    reply(F("PING         node name"));
  } else if (strncasecmp(cmd, "LCD:", 4) == 0) {
    printText(trim(cmd + 4));
  } else {
    printText(cmd);                     // plain text goes straight to the LCD
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
  lcd.init();
  lcd.backlight();
  showOnLcd("BT-LCD ready");
  reply(F("BT-LCD ready. Send HELP."));
}

void loop() {
  pollStream(bt, btBuf, btLen);         // from phone
  pollStream(Serial, usbBuf, usbLen);   // from Serial Monitor
}
