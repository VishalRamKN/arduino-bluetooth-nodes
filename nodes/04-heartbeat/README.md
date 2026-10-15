# Node 4 - Heartbeat (BT-HEART)

`HR` asks for the current BPM; `HRON` makes the node message the phone on
every detected beat. The on-board LED (D13) flashes with each beat.
Sketch: `BT_HEART/BT_HEART.ino`. No extra libraries.
Base kit wiring (UNO + HC-05): see [SETUP.md](../../SETUP.md), step 4.

> **Not a medical device.** Readings change with finger pressure and
> movement. Do not use them for any health decision.

## Pins (3-pin analog pulse sensor)

| Sensor pin | UNO pin |
| --- | --- |
| + (VCC) | 5V |
| - (GND) | GND |
| S (signal) | A0 |

## Commands

| Send | Does | Reply |
| --- | --- | --- |
| `HR` | Heart rate now | `Heart rate: 74 BPM` / `No pulse yet - rest finger on sensor` |
| `HRON` | Message on every beat | `Beat! BPM: 74` |
| `HROFF` | Stop beat messages | `Auto heartbeat: OFF` |
| `STATUS` | BPM, finger, auto mode | `HR: 74 BPM \| finger: yes \| auto: ON` |
| `PING` | Node name | `BT-HEART here` |
| `HELP` | Command list | |

## How it works

A0 is read every 10 ms. The sketch tracks the signal's peak and trough;
each rise above their midpoint is one beat. Gaps under 300 ms or over
2000 ms are ignored as noise. BPM = 60000 / average of the last 5 gaps.

## Test

Rest a fingertip lightly on the sensor and keep still. Within 5-10 s the D13
LED flashes with your pulse. Send `HR`, then `HRON`.
Always `No pulse yet`? Lower `MIN_SWING` from 40 to 20 in the sketch.
