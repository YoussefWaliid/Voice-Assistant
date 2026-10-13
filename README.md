# Voice-Assistant
a better way to Control your home 

An offline-wake-word, Arabic-friendly **voice-controlled home automation hub** running on an ESP32-S3.
Say the wake word, speak a command (Arabic or English), and the device runs a chain of actions:
HTTP calls, Wake-on-LAN, IR signals, relays, a smart power strip, and timed delays.
Everything is configured from a built-in web page, with no app and no cloud account for the device itself.

> **بالعربي باختصار:** جهاز ESP32-S3 بيسمع كلمة التنبيه (Alexa مثلاً)، تقوله أمر بصوتك، وهو ينفّذ سلسلة خطوات: ريلاي، IR، Wake-on-LAN، مشترك ذكي، HTTP، مع تأخير بين الخطوات. كل الإعدادات من صفحة ويب على الجهاز نفسه.

---

## Table of contents

- [Features](#features)
- [How it works](#how-it-works)
- [Hardware](#hardware)
- [Wiring](#wiring)
- [Project files](#project-files)
- [Setup](#setup)
- [Configuration](#configuration)
- [Using the web UI](#using-the-web-ui)
- [Commands and steps](#commands-and-steps)
- [How delays work](#how-delays-work)
- [Backup and restore](#backup-and-restore)
- [IR learning](#ir-learning)
- [Smart power strip (MTTL-W01)](#smart-power-strip-mttl-w01)
- [OTA updates](#ota-updates)
- [HTTP API](#http-api)
- [Troubleshooting](#troubleshooting)
- [Security notes](#security-notes)
- [Credits](#credits)

---

## Features

- **Local wake word** detection with Espressif **ESP-SR / WakeNet** (`alexa`, `jarvis`, or `hiesp`). Nothing is streamed until the wake word fires.
- **Speech-to-text** through [Wit.ai](https://wit.ai) for the command itself (Arabic and English).
- **Smart voice activity detection**: recording stops when you stop talking, with adjustable sensitivity.
- **Fault-tolerant phrase matching** in three passes: longest saved phrase contained in the speech, then all words in any order, then fuzzy matching for small transcription mistakes. Arabic text is normalized before matching.
- **Multi-step commands** with six step types: `HTTP`, `WOL`, `IR`, `Strip`, `Delay`, `Relay`.
- **Non-blocking delays** between steps, so the device keeps listening while a command waits.
- **Reorder steps** by drag and drop (mouse or touch) or with up/down buttons.
- **Import / export** all commands and learned IR signals to a JSON file.
- **IR learn and send** (raw capture, any remote) with configurable TX/RX pins.
- **2-channel relay** control and a **MTTL-W01 smart strip** controller (4 outlets, with power, energy and temperature readings).
- **Voice feedback** through an I2S amplifier, plus an RGB status LED.
- **Automatic retry**: if it did not hear or understand you, it replies and reopens the mic without needing the wake word again.
- **Live Serial Monitor** in the browser, a "what the device heard" log, and a text box to test commands without speaking.
- **OTA updates** from the Arduino IDE or from a browser (`/update`).
- Responsive **RTL Arabic web UI** with dark mode.

## How it works

```
 wake word (local, ESP-SR)
        │
        ▼
 record until silence (VAD)  ──►  Wit.ai speech-to-text
        │
        ▼
 normalize text ──► match against saved phrases ──► run the command's steps in order
                                                         │
              ┌───────────┬───────────┬──────────┬──────┴───┬─────────┐
              ▼           ▼           ▼          ▼          ▼         ▼
            HTTP         WOL          IR       Relay      Strip     Delay
```

Commands, phrases, IR signals and settings are stored in the device's flash (LittleFS). No external database.

## Hardware

| Part | Notes |
|------|-------|
| **ESP32-S3** board with PSRAM and an on-board RGB LED | ESP-SR needs an S3. Uses the `RGB_BUILTIN` LED. |
| **INMP441** I2S microphone | Input |
| **MAX98357A** I2S amplifier + small speaker | Voice replies |
| **2-channel relay module** (optional) | Opto-isolated with separate JD-VCC recommended |
| **IR LED** + transistor (optional) | Transmit |
| **VS1838B / TSOP** IR receiver (optional) | Learn signals |
| **MTTL-W01** smart strip (optional) | Controlled over the local network |

## Wiring

**Microphone (INMP441)**

| INMP441 | ESP32-S3 |
|---------|----------|
| WS | GPIO 15 |
| SD | GPIO 17 |
| SCK | GPIO 16 |
| VDD / GND | 3.3V / GND |

**Amplifier (MAX98357A)**

| MAX98357A | ESP32-S3 |
|-----------|----------|
| BCLK | GPIO 5 |
| LRC | GPIO 7 |
| DIN | GPIO 8 |
| VIN / GND | 5V / GND |
| SD, GAIN | leave unconnected (GAIN floating = 9 dB) |

**Relays**

| Relay module | ESP32-S3 |
|--------------|----------|
| IN1 | GPIO 1 |
| IN2 | GPIO 2 |
| VCC | 3.3V |
| JD-VCC | 5V from a separate charger (remove the jumper for full isolation) |
| GND | common ground |

Set `RELAY_ACTIVE_LOW` to `0` in the sketch if your module switches on a HIGH signal.

**IR** (defaults; changeable from the web UI): TX = GPIO 4, RX = GPIO 6.
Allowed pins: `1, 2, 4, 6, 9, 10, 11, 12, 13, 14, 18, 21, 38, 39, 40, 41, 42, 47`.
Do **not** reuse the relay, mic or amplifier pins for IR. Drive the IR LED through a transistor for useful range.

## Project files

| File | Purpose |
|------|---------|
| `Test_V5_1.ino` | Main firmware: audio, wake word, Wit.ai, command matching/execution, web server, OTA |
| `index_html.h` | The whole web UI (HTML/CSS/JS) as a PROGMEM string |
| `ir_module.h` | IR learn/send and its HTTP routes |
| `strip_module.h` | MTTL-W01 smart-strip controller |
| `audio_clips.h` | Pre-recorded voice replies (PCM) |

> Keep all files in the same sketch folder. The web UI lives in its own header because the Arduino preprocessor struggles with very long raw strings inside `.ino` files.

## Setup

### 1. Prerequisites

- Arduino IDE (or arduino-cli) with the **ESP32 Arduino core 3.x** (the code uses the newer `ledcAttach` API and ESP-SR model APIs).
- Libraries:
  - `ArduinoJson` (v6)
  - `WakeOnLan`
  - `WiFi`, `WebServer`, `LittleFS`, `HTTPClient`, `ArduinoOTA`, `Update` (bundled with the core)
- A free [Wit.ai](https://wit.ai) app and its **Server Access Token**.

### 2. Board settings

Select your ESP32-S3 board, enable **PSRAM**, and choose a **partition scheme that includes a `model` partition** (needed by ESP-SR) and enough flash for the app and LittleFS. The wake word model must be present in that partition, otherwise the serial log will print `Model partition not found!` or list the available models.

### 3. Configure and flash

1. Edit the settings at the top of `Test_V5_1.ino` (see [Configuration](#configuration)).
2. Upload over USB the first time.
3. Open the Serial Monitor at `115200` to see the device IP.
4. Browse to `http://<device-ip>/`.

## Configuration

All in `Test_V5_1.ino`:

| Setting | Default | Description |
|---------|---------|-------------|
| `ssid`, `password` | | Your Wi-Fi |
| `WIT_AI_TOKEN` | | Wit.ai server access token |
| `WAKE_WORD` | `"alexa"` | `jarvis`, `alexa`, or `hiesp` |
| `MIC_SHIFT` | `12` | Wake-word gain: 14 = original, 13 = 2x, 12 = 4x, 11 = 8x |
| `RECORD_TIME_SECONDS` | `6` | Max recording length (stops earlier on silence) |
| `VAD_SILENCE_MS` | `700` | Silence that ends a command |
| `VAD_NO_SPEECH_MS` | `3000` | Give up if nobody speaks |
| `REC_TARGET_PEAK`, `REC_MAX_GAIN` | `24000`, `400` | Recording normalization |
| `SPK_VOLUME` | `80` | Speaker volume (0-100) |
| `RETRY_MAX` | `1` | Extra listen attempts after a failure (0 disables) |
| `RELAY_ACTIVE_LOW` | `1` | Relay polarity |
| `STANDBY_LED_MS` | `60000` | Blue standby LED duration after last activity |
| `OTA_HOSTNAME` | `voice-assistant` | OTA / network name |
| `OTA_PASSWORD` | `12345678` | **Change this.** OTA and `/update` password (user `admin`) |

The microphone gain and VAD speech threshold can also be tuned live from the web UI (Maintenance tab) and are saved to flash.

## Using the web UI

Open `http://<device-ip>/`. Four tabs:

- **الأوامر (Commands)**: create/edit/run/delete commands, grouped by category. A collapsible panel shows the last phrases the device heard, with a **+** button to add a mis-heard phrase to a command.
- **IR**: choose TX/RX pins, learn new signals, test them, rename or delete saved signals.
- **المشترك (Strip)**: live status of the smart strip and per-outlet switches with power/energy/temperature.
- **الصيانة (Maintenance)**: device info, microphone tuning, backup/restore, live Serial Monitor (pause, clear, copy, download), a text box to simulate spoken commands, and a reboot button.

## Commands and steps

A command is a name, a category, one or more **spoken phrases** (all trigger the same command), and an ordered list of **steps**.

| Step type | Value format | Example |
|-----------|--------------|---------|
| `http` | URL (GET request) | `http://192.168.1.36/TOGGLE` |
| `wol` | MAC address | `D8:5E:D3:C2:A1:F8` |
| `ir` | ID of a saved IR signal (chosen from a dropdown in the UI) | `3` |
| `relay` | `channel:action`, action = `on` / `off` / `toggle` | `1:off`, `2:on` |
| `strip` | `outlet:action`, outlet `0` = all, `1`-`4` | `2:on`, `0:off` |
| `delay` | seconds (0.1 to 3600) | `10` |

Stored format (`/commands.json`):

```json
[
  {
    "id": 1,
    "name": "Play mode",
    "cat": "Automation",
    "phrases": ["ابدأ اللعب", "start gaming"],
    "steps": [
      { "type": "strip", "val": "2:on" },
      { "type": "delay", "val": "6.0" },
      { "type": "wol",   "val": "D8:5E:D3:C2:A1:F8" },
      { "type": "relay", "val": "2:on" },
      { "type": "relay", "val": "1:off" },
      { "type": "strip", "val": "1:on" },
      { "type": "delay", "val": "10.0" },
      { "type": "ir",    "val": "1" }
    ]
  }
]
```

To reorder steps, drag the **⠿** handle or use the **▲ ▼** buttons next to each step, then save.

## How delays work

Steps run top to bottom. A `delay` step pauses **only the steps after it**, and the countdown starts the moment the step right before it finishes. It is not measured from the start of the command or from the time you spoke.

In the example above:

1. `strip 2:on` runs immediately.
2. After 6 s: WOL, relay 2 on, relay 1 off, strip 1 on.
3. Right after `strip 1:on`, a 10 s timer starts, then the IR signal is sent.

Details:

- Delays are **non-blocking**: the rest of the command is scheduled and the main loop keeps serving the web UI, wake word, etc.
- Up to **4 delayed commands** can wait at once.
- A `delay` as the **last** step is ignored.
- The confirmation voice reply plays after the first segment (before the first delay) finishes.
- If you trigger the wake word while a delay is pending, the recording takes a few seconds of the main loop and the delayed steps may start slightly late.

## Backup and restore

**Maintenance tab → Backup**

- **Export** downloads `voice-hub-backup-YYYY-MM-DD.json` containing all commands (phrases, steps, order) **and all learned IR signals** (raw pulses).
- **Import** can either **replace** everything or **add** to what exists. IR signals are re-created first and the IR steps inside commands are automatically re-mapped to the new signal IDs.
- Older exports that contain only commands still import fine.

Make a backup before OTA updates and before clearing flash.

## IR learning

1. IR tab → pick TX and RX GPIOs → save.
2. Press **Start learning**, point the remote at the receiver, press one button (15 s timeout).
3. Test it with **▶**, set carrier (36/38/40/56 kHz) and repeat count, name it, save.
4. Use it as an `ir` step in any command.

Signals are stored raw (pulse durations) so almost any remote works. Max length is 600 edges per signal.

## Smart power strip (MTTL-W01)

The ESP32 acts as the **controller** for an MTTL-W01 strip. In the vendor's **MTTL Control** app go to *Device setup* and enter the IP shown on the Strip tab (port `10086`). The tab then shows connection state, MAC, firmware version and, per outlet, watts, kWh and temperature, with on/off controls and "all on / all off".

## OTA updates

> The `.h` files are part of the firmware. After changing any of them, rebuild the sketch and upload again.

- **Arduino IDE:** the device appears under *Tools → Port → Network ports* as `voice-assistant`. Upload as usual and enter the OTA password.
- **Browser:** *Sketch → Export Compiled Binary*, then open `http://<device-ip>/update`, log in (`admin` / your `OTA_PASSWORD`) and upload the app `.bin` (not the bootloader or partitions file). The device reboots by itself.

Saved commands and IR signals live in LittleFS and survive OTA updates.

## HTTP API

The UI is a thin client over these endpoints. Useful for scripting or integrating with other systems.

| Method | Endpoint | Description |
|--------|----------|-------------|
| GET | `/api/commands` | All commands (JSON) |
| POST | `/api/save` | Create/update a command (JSON body, `id: 0` = new) |
| POST | `/api/delete` | Delete by `id` |
| GET / POST | `/api/run?id=N` | Run a command by `id` (the web UI shows a copyable link under each command, for HTTP Shortcuts) |
| POST | `/api/clear` | Delete all commands |
| POST | `/api/say` | Simulate a spoken sentence: `text=...` |
| GET/POST | `/api/relay` | `?n=1&s=on\|off\|toggle` (no `s` returns state) |
| GET | `/api/ir/cfg`, POST | Get / set IR pins |
| POST | `/api/ir/learn`, GET `/api/ir/status` | Start learning / poll result |
| GET | `/api/ir/list` | Saved IR signals (metadata) |
| POST | `/api/ir/save`, `/api/ir/send`, `/api/ir/delete` | Save, send, delete |
| GET | `/api/ir/raw?id=` | Raw pulses of a signal (backup) |
| POST | `/api/ir/import?name=&khz=&rep=` | Create a signal from comma-separated pulses in the body |
| GET | `/api/strip/status` | Smart strip state |
| POST | `/api/strip/set`, `/api/strip/refresh` | Switch outlet(s) / refresh readings |
| GET/POST | `/api/mic` | Mic gain and VAD level |
| GET/POST | `/api/heard/cfg`, GET `/api/heard`, POST `/api/heard/clear` | "Heard" log |
| GET | `/api/log?since=` , POST `/api/log/clear` | Serial log buffer |
| GET | `/api/info` | IP, RSSI, heap, PSRAM, uptime, wake word |
| POST | `/api/reboot` | Restart |
| GET/POST | `/update` | Browser OTA (basic auth) |

## Troubleshooting

| Symptom | What to check |
|---------|---------------|
| `Model partition not found!` | Use a partition scheme with a `model` partition and flash the WakeNet model |
| `Wake word model not found!` | The serial log lists available models; set `WAKE_WORD` to one of them |
| Doesn't wake from a distance | Increase wake gain (`MIC_SHIFT` 12 → 11) or raise the mic gain in the Maintenance tab |
| Cuts off your sentence / ignores quiet speech | Lower "minimum speech level" in the Maintenance tab; raise `VAD_SILENCE_MS` |
| "No network" voice reply | Wi-Fi or Wit.ai problem, check the token and internet access |
| Phrase not matched | Open "last things the device heard" and add the exact transcription with **+** |
| Relays are inverted | Toggle `RELAY_ACTIVE_LOW` |
| IR does nothing | Check TX pin choice, use a transistor, raise the repeat count, re-learn the signal |
| Network port not visible for OTA | Same network/subnet, restart the IDE, check firewall (mDNS/UDP) |

## Security notes

- **Never commit your real Wi-Fi password, Wit.ai token, or OTA password.** Keep placeholders in the repository (for example in a `secrets.h` that is listed in `.gitignore`). If a token was ever committed, **revoke it in Wit.ai and generate a new one**.
- The web UI and `/api/*` endpoints have **no authentication**. Only expose the device on a trusted LAN, never port-forward it to the internet.
- Change the default `OTA_PASSWORD`.
- Relays may switch mains-powered loads. Use properly rated, isolated modules and enclosures, and take care with mains wiring.

## Credits

- [Espressif ESP-SR / WakeNet](https://github.com/espressif/esp-sr)
- [Wit.ai](https://wit.ai) speech API
- [ArduinoJson](https://arduinojson.org), [WakeOnLan](https://github.com/a7md0/WakeOnLan-ESP32)

## License

Add the license of your choice (for example MIT) as a `LICENSE` file.
