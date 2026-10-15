# Setup Guide

Do these steps in order. Each step tells you what you should see when it
worked. If something does not match, see **Troubleshooting** at the end.

## 1. Install the Arduino IDE

1. Download **Arduino IDE 2** from https://www.arduino.cc/en/software and install it.
2. Plug the Arduino UNO into your computer with a USB cable.
3. **Tools → Board → Arduino AVR Boards → Arduino Uno**.
4. **Tools → Port** → pick the port that appeared when you plugged in the UNO
   (Windows: `COM3`, `COM4`...; macOS: `/dev/cu.usbmodem...`; Linux: `/dev/ttyACM0`).

Check: "Arduino Uno" shows in the board box at the top of the IDE.

## 2. Install the LCD library (only if you use the LCD)

1. **Sketch → Include Library → Manage Libraries**.
2. Search **LiquidCrystal I2C**.
3. Install the one **by Frank de Brabander**.

## 3. Get the code

With Git:

    git clone <REPOSITORY-URL>

Without Git: on the repository page click **Code → Download ZIP** and unzip it.

Open a sketch with **File → Open** and choose its `.ino` file,
for example `nodes/01-lcd/BT_LCD/BT_LCD.ino`.

## 4. Wire the base kit (UNO + HC-05)

This is the same for the combined hub and for every node.

| HC-05 pin | Arduino UNO pin | Notes |
| --- | --- | --- |
| VCC | 5V | |
| GND | GND | |
| TXD | D2 | Direct wire |
| RXD | D3 | **Through the voltage divider below** |
| STATE, EN | not connected | |

    HC-05                         Arduino UNO
    -----                         -----------
    VCC  ------------------------ 5V
    GND  ------------------------ GND
    TXD  ------------------------ D2
    RXD  ----+-------[ 1k ]------ D3
             |
           [ 2k ]
             |
            GND

**Why the divider?** D3 outputs 5 V but HC-05 RXD accepts only 3.3 V.
1 kΩ and 2 kΩ drop 5 V to about 3.3 V (5 x 2 / 3 = 3.33 V).
TXD to D2 needs no divider because the UNO reads 3.3 V as HIGH.

**Why D2/D3 instead of D0/D1?** D0/D1 are used by USB. With the HC-05 on
D2/D3 you can upload code without unplugging it.

Then wire your component(s):

| Component | Pins |
| --- | --- |
| I2C LCD | GND→GND, VCC→5V, SDA→A4, SCL→A5 |
| HC-SR04 | VCC→5V, Trig→D7, Echo→D6, GND→GND |
| SG90 servo | Brown→GND, Red→5V, Orange→D9; 470 µF capacitor across red/brown (stripe to brown) |
| Pulse sensor | +→5V, -→GND, S→A0 |
| LED | Long leg → 220 Ω → D12, short leg → GND |

**Combined hub only:** the servo can draw several hundred mA. Power it from a
separate 5 V supply (1 A or more) and connect that supply's GND to UNO GND.

## 5. Rename each HC-05 (five-node setup only)

Every HC-05 is called "HC-05" from the factory, so give each a unique name.

| Node | Name |
| --- | --- |
| 1 LCD | `BT-LCD` |
| 2 Ultrasonic | `BT-DIST` |
| 3 Servo | `BT-SERVO` |
| 4 Heartbeat | `BT-HEART` |
| 5 LED | `BT-LED` |

1. Open and upload `tools/HC05_AT_Mode/HC05_AT_Mode.ino`.
2. Unplug the HC-05 **VCC** wire.
3. **Press and hold** the small button on the HC-05, plug VCC back in, then release.
   Check: the HC-05 LED blinks **slowly** (about once every 2 s). This is AT mode.
4. **Tools → Serial Monitor**, set **9600 baud** and **Both NL & CR**.
5. Type `AT`, press Enter. Check: reply `OK`.
6. Type `AT+NAME=BT-LCD` (the right name for this node). Check: `OK`.
   If you get an error, use quotes: `AT+NAME="BT-LCD"`.
7. Type `AT+UART?`. Check: `+UART:9600,0,0`. If not, send `AT+UART=9600,0,0`.
8. Optional: new pairing PIN with `AT+PSWD=4321` (or `AT+PSWD="4321"`).
9. Unplug and re-plug the HC-05 **without** the button.
   Check: the LED blinks **fast** again (normal mode).

Repeat for every module and stick a label on each board.

## 6. Upload the sketch

1. Open the sketch:
   - Combined hub: `combined-hub/BT_Hub_All/BT_Hub_All.ino`
   - Nodes: `nodes/01-lcd/BT_LCD/BT_LCD.ino`, `nodes/02-ultrasonic/BT_DIST/BT_DIST.ino`, and so on
2. Click **Upload** (right-arrow button). Check: "Done uploading".
3. Open **Serial Monitor** (9600 baud, Both NL & CR).
   Check: a "ready" message such as `BT-LCD ready. Send HELP.`
4. Type `HELP` to test without the phone.

## 7. Connect your phone

1. Power the board(s). The HC-05 LED blinks fast (waiting for a phone).
2. Phone **Settings → Bluetooth** → pair the module (`HC-05`, or `BT-LCD`, `BT-DIST`... if renamed).
   PIN: `1234` (some modules use `0000`).
3. Install **Serial Bluetooth Terminal** (by Kai Morich) from the Play Store,
   or any Bluetooth Classic terminal app.
4. Allow the **Nearby devices** permission (Android 12 and newer).
5. In the app: **Settings → Send → Newline: CR+LF**.
   Without this the Arduino never sees the end of your command.
6. Menu → **Devices** → **Bluetooth Classic** → tap your module.
   Check: the app says "Connected" and the HC-05 LED double-blinks slowly.
7. Send `HELP`. Check: the command list arrives.

**Five nodes:** the phone talks to one node at a time. To switch, disconnect,
go back to Devices and tap another name. Send `PING` to see which node you are on.

**Tip:** long-press the macro buttons (M1, M2...) to save commands like
`DIST`, `ON`, `OFF`, `HR`, `STATUS`.

## 8. Test checklist

| Send | Expected reply | Expected hardware |
| --- | --- | --- |
| `HELP` | Command list | - |
| `LCD:Hello` (hub) or `Hello` (node) | `LCD: shown` | "Hello" on the LCD |
| `DIST` | `Distance: 20.1 cm` | Hand held 20 cm away |
| `SERVO:0`, then `SERVO:180` | `Servo moved to ... deg` | Servo sweeps end to end |
| `HR` (finger resting 10 s) | `Heart rate: 72 BPM` | - |
| `HRON` | `Beat! BPM: 72` about once a second | - |
| `LED:ON` (hub) or `ON` (node) | `LED: ON` | LED lights |
| `STATUS` | Current state | - |

## 9. Troubleshooting

| Problem | Likely cause | Fix |
| --- | --- | --- |
| App connects but nothing replies | No line ending | Set newline to **CR+LF** in the app |
| Garbled characters | HC-05 speed is not 9600 | AT mode → `AT+UART=9600,0,0` |
| Arduino receives, phone gets nothing | Divider or D3 wiring | D3 → 1 kΩ → RXD, RXD → 2 kΩ → GND |
| `AT` gets no `OK` | Not in AT mode | LED must blink slowly; repeat step 5.3 |
| All modules show as "HC-05" | Not renamed | Do step 5, then unpair and re-pair on the phone |
| LCD lit but no text | Wrong I2C address or contrast | Set `LCD_ADDR` to `0x3F`; turn the blue contrast screw |
| `Distance: out of range` every time | Trig/Echo swapped | Trig → D7, Echo → D6 |
| Arduino resets when the servo moves | Power dip | Add 470 µF capacitor; separate 5 V supply with shared GND |
| Servo twitches slightly while typing | SoftwareSerial timing | Normal and harmless |
| `No pulse yet` every time | Finger moving or weak signal | Rest finger lightly and still for 10 s; lower `MIN_SWING` to 20 |
| Upload fails: port busy / not found | Wrong port or another program has it | Close other serial apps; re-select **Tools → Port** |
| `LiquidCrystal_I2C.h: No such file` | Library missing | Do step 2 |
