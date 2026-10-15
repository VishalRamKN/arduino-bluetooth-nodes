# Node 2 - Ultrasonic (BT-DIST)

Send `DIST`: the node takes three readings and replies with the middle one in cm.
Sketch: `BT_DIST/BT_DIST.ino`. No extra libraries.
Base kit wiring (UNO + HC-05): see [SETUP.md](../../SETUP.md), step 4.

## Pins

| HC-SR04 pin | UNO pin |
| --- | --- |
| VCC | 5V |
| Trig | D7 |
| Echo | D6 |
| GND | GND |

## Commands

| Send | Does | Reply |
| --- | --- | --- |
| `DIST` or `D` | Measures distance | `Distance: 23.4 cm` / `Distance: out of range` |
| `STATUS` | Last reading and its age | `Last: 23.4 cm, 12 s ago` |
| `PING` | Node name | `BT-DIST here` |
| `HELP` | Command list | |

## How it works

A 10 µs pulse on Trig makes the sensor send an ultrasonic burst; Echo stays
HIGH until the sound returns. Distance = time x 0.0343 / 2 (cm). Range is
about 2 to 400 cm.

## Test

Hold a book 20 cm away and send `DIST`; the reply should be close to 20 cm.
