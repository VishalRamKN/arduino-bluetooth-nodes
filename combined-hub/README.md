# Combined Hub

One Arduino UNO + one HC-05 controlling all five components at once.
Sketch: `BT_Hub_All/BT_Hub_All.ino`. Library needed: LiquidCrystal I2C.
Base kit wiring (UNO + HC-05): see [SETUP.md](../SETUP.md), step 4.

## Pins

| Component | Pin | UNO |
| --- | --- | --- |
| HC-05 | VCC / GND / TXD / RXD | 5V / GND / D2 / D3 via 1k-2k divider |
| I2C LCD | GND / VCC / SDA / SCL | GND / 5V / A4 / A5 |
| HC-SR04 | VCC / Trig / Echo / GND | 5V / D7 / D6 / GND |
| SG90 servo | Brown / Red / Orange | GND / 5V (external recommended) / D9 |
| Pulse sensor | + / - / S | 5V / GND / A0 |
| LED | Long leg / short leg | D12 via 220 Ω / GND |

## Commands

| Send | Does | Reply |
| --- | --- | --- |
| `LCD:<text>` | Shows text (16 chars per line, 32 max) | `LCD: shown` |
| `LCDCLR` | Clears LCD | `LCD: cleared` |
| `DIST` | Measures distance | `Distance: 23.4 cm` |
| `SERVO:<0-180>` | Turns servo | `Servo moved to 90 deg` |
| `SERVO?` | Servo angle | `Servo is at 90 deg` |
| `HR` | Heart rate now | `Heart rate: 74 BPM` |
| `HRON` / `HROFF` | Message on every beat on/off | `Beat! BPM: 74` |
| `LED:ON` / `LED:OFF` | LED on/off | `LED: ON` |
| `STATUS` | Everything | `Servo: 90 deg \| LED: OFF \| HR: 74 BPM \| Auto HR: OFF` |
| `HELP` | Command list | |

## Power

Power the servo from a separate 5 V supply (1 A+) with its GND joined to UNO
GND, and put a 470 µF capacitor across the servo supply. Otherwise the UNO
may reset when the servo moves.
