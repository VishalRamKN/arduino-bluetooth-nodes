/* Node 4 - BT-HEART : Arduino UNO + HC-05 + analog pulse sensor
   Not a medical device.
   License: MIT (see LICENSE) */
#include <SoftwareSerial.h>
#include <string.h>

const char NODE_NAME[] = "BT-HEART";
const uint8_t PULSE_PIN = A0;
const uint8_t BLINK_PIN = 13;       // on-board LED flashes on each beat

SoftwareSerial bt(2, 3);            // D2 <- HC-05 TXD, D3 -> HC-05 RXD (divider)

const uint8_t CMD_MAX = 64;
char btBuf[CMD_MAX + 1];  uint8_t btLen = 0;
char usbBuf[CMD_MAX + 1]; uint8_t usbLen = 0;

// ---- detection settings ----
const unsigned long SAMPLE_MS  = 10;    // 100 samples per second
const int           MIN_SWING  = 40;    // smaller = no finger / just noise
const unsigned long MIN_IBI    = 300;   // faster than 200 BPM = noise
const unsigned long MAX_IBI    = 2000;  // slower than 30 BPM = noise
const unsigned long TIMEOUT_MS = 3000;  // no beat for 3 s = pulse lost

// ---- detection state ----
unsigned long lastSample = 0, lastBeat = 0;
int  peak = 512, trough = 512;
bool above = false, fingerOn = false, autoMode = false;
unsigned long ibi[5];
uint8_t ibiCount = 0, ibiIndex = 0;
int bpm = 0;

void reply(const char *m)                { bt.println(m); Serial.println(m); }
void reply(const __FlashStringHelper *m) { bt.println(m); Serial.println(m); }

char *trim(char *s) {
  while (*s == ' ') s++;
  size_t n = strlen(s);
  while (n > 0 && s[n - 1] == ' ') s[--n] = '\0';
  return s;
}

// ---- runs every loop, never blocks ----
void updateHeartbeat() {
  unsigned long now = millis();
  if (now - lastSample < SAMPLE_MS) return;
  lastSample = now;

  int v = analogRead(PULSE_PIN);

  // envelope: jumps to new highs/lows, then relaxes 1 step per sample
  peak   = (v > peak)   ? v : peak - 1;
  trough = (v < trough) ? v : trough + 1;
  int swing = peak - trough;
  int mid   = (peak + trough) / 2;
  int band  = swing / 8;              // hysteresis
  fingerOn  = swing >= MIN_SWING;

  if (!fingerOn) {
    above = false;
  } else if (!above && v > mid + band) {   // rising edge = beat
    above = true;
    digitalWrite(BLINK_PIN, HIGH);
    unsigned long gap = now - lastBeat;
    if (gap >= MIN_IBI) {
      lastBeat = now;
      if (gap <= MAX_IBI) {
        ibi[ibiIndex] = gap;
        ibiIndex = (ibiIndex + 1) % 5;
        if (ibiCount < 5) ibiCount++;
        unsigned long sum = 0;
        for (uint8_t i = 0; i < ibiCount; i++) sum += ibi[i];
        bpm = 60000UL / (sum / ibiCount);
        if (autoMode && ibiCount >= 2) {
          char out[24];
          snprintf(out, sizeof(out), "Beat! BPM: %d", bpm);
          reply(out);
        }
      }
    }
  } else if (above && v < mid - band) {    // signal fell back
    above = false;
    digitalWrite(BLINK_PIN, LOW);
  }

  if (bpm > 0 && now - lastBeat > TIMEOUT_MS) {
    bpm = 0; ibiCount = 0; ibiIndex = 0;
    digitalWrite(BLINK_PIN, LOW);
    if (autoMode) reply(F("Pulse lost - rest finger on sensor"));
  }
}

void handleCommand(char *cmd) {
  cmd = trim(cmd);
  if (*cmd == '\0') return;
  char out[56];

  if (strcasecmp(cmd, "HR") == 0) {
    if (bpm == 0) reply(F("No pulse yet - rest finger on sensor"));
    else {
      snprintf(out, sizeof(out), "Heart rate: %d BPM%s", bpm,
               ibiCount < 3 ? " (settling)" : "");
      reply(out);
    }
  } else if (strcasecmp(cmd, "HRON") == 0) {
    autoMode = true;
    reply(F("Auto heartbeat: ON"));
  } else if (strcasecmp(cmd, "HROFF") == 0) {
    autoMode = false;
    reply(F("Auto heartbeat: OFF"));
  } else if (strcasecmp(cmd, "STATUS") == 0) {
    snprintf(out, sizeof(out), "HR: %d BPM | finger: %s | auto: %s",
             bpm, fingerOn ? "yes" : "no", autoMode ? "ON" : "OFF");
    reply(out);
  } else if (strcasecmp(cmd, "PING") == 0) {
    snprintf(out, sizeof(out), "%s here", NODE_NAME);
    reply(out);
  } else if (strcasecmp(cmd, "HELP") == 0) {
    reply(F("BT-HEART commands:"));
    reply(F("HR          heart rate now"));
    reply(F("HRON        message on every beat"));
    reply(F("HROFF       stop beat messages"));
    reply(F("STATUS      BPM, finger, auto mode"));
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
  pinMode(BLINK_PIN, OUTPUT);
  reply(F("BT-HEART ready. Send HELP."));
}

void loop() {
  pollStream(bt, btBuf, btLen);
  pollStream(Serial, usbBuf, usbLen);
  updateHeartbeat();
}
