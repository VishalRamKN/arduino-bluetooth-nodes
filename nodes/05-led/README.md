# Node 5 - LED (BT-LED)

Send `ON` or `OFF`; the LED follows and `STATUS` reports its state.
Sketch: `BT_LED/BT_LED.ino`. No extra libraries.
Base kit wiring (UNO + HC-05): see [SETUP.md](../../SETUP.md), step 4.

## Pins

| LED leg | UNO pin |
| --- | --- |
| Anode (long leg) | D12 through a 220 Ω resistor |
| Cathode (short leg, flat side) | GND |

## Commands

| Send | Does | Reply |
| --- | --- | --- |
| `ON`, `LED:ON` or `1` | LED on | `LED: ON` |
| `OFF`, `LED:OFF` or `0` | LED off | `LED: OFF` |
| `STATUS` | LED state | `LED is OFF` |
| `PING` | Node name | `BT-LED here` |
| `HELP` | Command list | |

## Test

Send `ON` (LED lights), `STATUS` (`LED is ON`), then `0` (LED off).
LED never lights? Turn it around: long leg goes to the resistor.
