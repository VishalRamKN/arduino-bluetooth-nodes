/*
  HC05_AT_Mode - pass-through for configuring an HC-05 (rename, baud, PIN)
  Wiring: HC-05 TXD -> D2, HC-05 RXD <- D3 (through 1k/2k divider)
  Use: put the HC-05 in AT mode (hold its button while powering it),
       open Serial Monitor at 9600 baud with "Both NL & CR", type AT.
  See SETUP.md, step 5.  License: MIT (see LICENSE)
*/
#include <SoftwareSerial.h>

SoftwareSerial bt(2, 3);          // D2 <- TXD, D3 -> RXD (through divider)

void setup() {
  Serial.begin(9600);             // Serial Monitor speed
  bt.begin(38400);                // HC-05 AT-mode speed
  Serial.println("AT mode ready. Type AT and press Enter.");
}

void loop() {
  if (bt.available())     Serial.write(bt.read());
  if (Serial.available()) bt.write(Serial.read());
}
