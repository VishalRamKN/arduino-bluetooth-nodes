# Arduino Bluetooth Nodes

Control an LCD, an ultrasonic sensor, a servo, a heartbeat sensor and an LED
from an Android phone using an **Arduino UNO** and an **HC-05 Bluetooth module**.
You type short text commands in a Bluetooth terminal app; the Arduino acts
on them and replies.

**New here? Start with [SETUP.md](SETUP.md).**

## Two ways to build it

| Option | What it is | Folder |
| --- | --- | --- |
| Combined hub | One UNO + one HC-05 with all five components | [combined-hub/](combined-hub/) |
| Five nodes | Five separate UNO + HC-05 boards, one component each | [nodes/](nodes/) |

## Repository layout

    arduino-bluetooth-nodes/
    ├── README.md            you are here
    ├── SETUP.md             step-by-step setup guide (start here)
    ├── LICENSE              MIT license (code)
    ├── LICENSE-DOCS.md      CC BY 4.0 license (documentation)
    ├── tools/
    │   └── HC05_AT_Mode/    sketch to rename an HC-05
    ├── combined-hub/
    │   └── BT_Hub_All/      all five components on one UNO
    └── nodes/
        ├── 01-lcd/          Node 1: BT-LCD
        ├── 02-ultrasonic/   Node 2: BT-DIST
        ├── 03-servo/        Node 3: BT-SERVO
        ├── 04-heartbeat/    Node 4: BT-HEART
        └── 05-led/          Node 5: BT-LED

Each sketch is inside a folder with the **same name** as its `.ino` file
(for example `BT_LCD/BT_LCD.ino`). The Arduino IDE requires this, so never
rename one without the other.

## Hardware

| Part | Combined hub | Five nodes |
| --- | --- | --- |
| Arduino UNO | 1 | 5 |
| HC-05 Bluetooth module | 1 | 5 |
| 1 kΩ resistor | 1 | 5 |
| 2 kΩ resistor | 1 | 5 |
| I2C LCD 16x2 (PCF8574 backpack) | 1 | 1 |
| HC-SR04 ultrasonic sensor | 1 | 1 |
| SG90 servo + 470 µF capacitor | 1 | 1 |
| Analog pulse sensor (pins +, -, S) | 1 | 1 |
| LED + 220 Ω resistor | 1 | 1 |
| Breadboard + jumper wires | 1 set | 5 sets |

## Pin map (same in every sketch)

| Component | Pins |
| --- | --- |
| HC-05 | VCC→5V, GND→GND, TXD→D2, RXD→D3 **through 1 kΩ/2 kΩ divider** |
| I2C LCD | GND→GND, VCC→5V, SDA→A4, SCL→A5 |
| HC-SR04 | VCC→5V, Trig→D7, Echo→D6, GND→GND |
| SG90 servo | Brown→GND, Red→5V, Orange→D9 |
| Pulse sensor | +→5V, -→GND, S→A0 |
| LED | Long leg → 220 Ω → D12, short leg → GND |

## Command summary

Commands are not case-sensitive and must end with a newline
(set your phone app to send **CR+LF**).

| Component | Combined hub | Five nodes |
| --- | --- | --- |
| LCD | `LCD:Hello`, `LCDCLR` | any text, `LCD:<text>`, `CLR` |
| Ultrasonic | `DIST` | `DIST` or `D` |
| Servo | `SERVO:90`, `SERVO?` | `90`, `SERVO:90`, `SERVO?` |
| Heartbeat | `HR`, `HRON`, `HROFF` | `HR`, `HRON`, `HROFF` |
| LED | `LED:ON`, `LED:OFF` | `ON`, `OFF`, `1`, `0` |
| Every sketch | `STATUS`, `HELP` | `STATUS`, `HELP`, `PING` |

## Libraries

| Library | How to get it |
| --- | --- |
| LiquidCrystal I2C (by Frank de Brabander) | Arduino IDE → Library Manager → search "LiquidCrystal I2C" |
| Wire, SoftwareSerial, Servo | Built into the Arduino IDE |

## Safety notes

- **HC-05 RXD is a 3.3 V pin.** Always use the 1 kΩ / 2 kΩ divider between
  Arduino D3 and HC-05 RXD.
- **The heartbeat sensor is a hobby part, not a medical device.** Do not use
  its readings for any health decision.

## License

- Code (`.ino` files): MIT License, see [LICENSE](LICENSE).
- Documentation (`.md` files): CC BY 4.0, see [LICENSE-DOCS.md](LICENSE-DOCS.md).
