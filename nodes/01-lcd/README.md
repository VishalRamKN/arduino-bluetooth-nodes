# Node 1 - LCD (BT-LCD)

Anything you type that is not a command appears on the LCD.
Sketch: `BT_LCD/BT_LCD.ino`. Library needed: LiquidCrystal I2C.
Base kit wiring (UNO + HC-05): see [SETUP.md](../../SETUP.md), step 4.

## Pins

| LCD pin | UNO pin |
| --- | --- |
| GND | GND |
| VCC | 5V |
| SDA | A4 |
| SCL | A5 |

## Commands

| Send | Does | Reply |
| --- | --- | --- |
| `Hello World` (any text) | Shows it: 16 chars line 1, next 16 line 2 | `LCD: shown` |
| `LCD:help` | Shows text that is also a command word | `LCD: shown` |
| `CLR` | Clears the screen | `LCD: cleared` |
| `STATUS` | What is on screen | `LCD shows: Hello World` |
| `PING` | Node name | `BT-LCD here` |
| `HELP` | Command list | |

## Test

Send `PING`, then `Hello World` (appears on the LCD), then `CLR`.
Blank screen with backlight on? Change `LCD_ADDR` to `0x3F` or turn the contrast screw.
