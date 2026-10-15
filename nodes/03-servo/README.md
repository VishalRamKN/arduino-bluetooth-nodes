# Node 3 - Servo (BT-SERVO)

Send an angle from 0 to 180 (just the number, or `SERVO:90`); `STATUS` reports it.
Sketch: `BT_SERVO/BT_SERVO.ino`. Uses the built-in Servo library.
Base kit wiring (UNO + HC-05): see [SETUP.md](../../SETUP.md), step 4.

## Pins

| SG90 wire | UNO pin | Notes |
| --- | --- | --- |
| Brown | GND | |
| Red | 5V | External 5 V supply is steadier (join its GND to UNO GND) |
| Orange | D9 | Control signal |

Put a **470 µF capacitor** across red and brown (stripe to brown). It stops
the UNO resetting when the servo starts moving.

## Commands

| Send | Does | Reply |
| --- | --- | --- |
| `90` (any 0-180) | Turns to that angle | `Servo moved to 90 deg` |
| `SERVO:45` | Same, long form | `Servo moved to 45 deg` |
| `STATUS` or `SERVO?` | Current angle | `Servo is at 45 deg` |
| `PING` | Node name | `BT-SERVO here` |
| `HELP` | Command list | |
| `200` or `abc` | Rejected | `Error: send a number 0 to 180` |

## Test

Send `0`, `180`, then `90`. The servo sweeps both ways and returns to centre.
