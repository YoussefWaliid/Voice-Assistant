# Voice Hub — ESP32-S3 Arabic Voice Assistant & Home Automation Hub

A self-contained smart-home controller running on an **ESP32-S3**. Say a wake word, speak an Arabic (or English) command, and the device executes a multi-step action: HTTP requests, Wake-on-LAN, IR codes, local relays, or a smart power strip. Everything — commands, phrases, IR codes, pin assignments — is managed from an on-device, phone-friendly web UI. **No firmware edits are needed to add or change a command.**

---

## Table of contents

1. [Features](#features)
2. [How it works](#how-it-works)
3. [Hardware](#hardware)
4. [Wiring](#wiring)
5. [Repository layout](#repository-layout)
6. [Build & flash](#build--flash)
7. [Configuration](#configuration)
8. [Web UI guide](#web-ui-guide)
9. [Commands, phrases and steps](#commands-phrases-and-steps)
10. [IR module](#ir-module)
11. [Power-strip module (MTTL-W01)](#power-strip-module-mttl-w01)
12. [Time & timestamps](#time--timestamps)
13. [HTTP API reference](#http-api-reference)
14. [Updating firmware (OTA)](#updating-firmware-ota)
15. [Troubleshooting](#troubleshooting)
16. [Security notes](#security-notes)

---

## Features

**Voice**
- Offline wake-word detection with **ESP-SR WakeNet** (`alexa` by default; `jarvis` / `hiesp` also supported).
- Voice-activity detection (VAD): recording stops when you stop talking, adapts to room noise, ignores clicks.
- Speech-to-text through **Wit.ai** (Arabic/English).
- Arabic text normalisation (diacritics, alef/yaa/taa-marbuta variants, Arabic-Indic digits, definite article “ال”) so phrase matching is forgiving.
- Three-stage matching: longest phrase contained → all words in any order → fuzzy per-word match (edit distance).
- Spoken replies from the **MAX98357A** I2S amplifier: random wake and “done” variants, plus “didn’t hear you”, “didn’t understand”, “error”, and “no internet” clips. A failed attempt re-opens the mic automatically (configurable) without the wake word.
- RGB LED status: cyan = wake detected, red = recording, purple = processing, green/orange/red = result, dim blue standby for 60 s then off.

**Automation**
- **Commands** with a category, any number of spoken phrases, and an ordered list of steps.
- Step types: `HTTP`, `WOL`, `IR`, `Strip` (power strip), `Relay`, and **`Delay`** (non-blocking — the rest of the command resumes later while the device keeps working).
- Every command also gets a copyable URL (`/api/run?id=N`) for shortcuts apps such as *HTTP Shortcuts*.
- 2-channel relay output (active-low, fully isolated supply).
- IR learn / send with raw pulse capture, adjustable carrier and repeat count.
- Full controller for the LG U+ / TONLY **MTTL-W01** power strip with live telemetry.

**Web UI** (RTL, Arabic, dark/light, installable PWA)
- Phone-first layout; circular tab swiping.
- Categorised, searchable, reorderable command list; drag-and-drop step ordering.
- Live power-strip dashboard (watts, amps, kWh, temperature, per-outlet control).
- Maintenance area with live **Serial Monitor**, device status, microphone tuning, IR tools, and backup/restore.

**Operations**
- Device-side **NTP clock** (Egypt time with automatic DST, 12-hour format) and millisecond **timestamps on every log line**.
- OTA updates from Arduino IDE or from the browser (`/update`).
- Wi-Fi watchdog with auto-reconnect.
- JSON backup/restore of commands **and** learned IR codes.

---

## How it works

```
 mic (INMP441) ──► WakeNet (on-device) ──► reply clip ──► VAD recording
                                                              │
                                                     Wit.ai speech-to-text
                                                              │
                         normalise Arabic ◄───────────────────┘
                                │
               match phrase → command → run steps in order
                                │
   ┌───────────┬───────────┬────┴─────┬─────────────┬───────────┐
  HTTP        WOL         IR       Relay        Strip        Delay
 (GET)   (magic packet) (38 kHz)  (GPIO 1/2)  (TCP :10086)  (job queue)
                                │
                  spoken confirmation + LED colour
```

The web server, IR learning, power-strip socket, delayed jobs and OTA are all serviced from `loop()`, so the device stays responsive while it waits on a delay.

---

## Hardware

| Part | Purpose |
|---|---|
| ESP32-S3 (N16R8 — 16 MB flash, 8 MB PSRAM) | Main controller |
| INMP441 | I2S MEMS microphone |
| MAX98357A + speaker | I2S amplifier for spoken replies |
| IR LED + NPN transistor | IR transmitter (driven by LEDC PWM carrier) |
| VS1838B / TSOP | IR receiver (output idles HIGH) |
| 2-channel 5 V relay module | Local switching (optional) |
| LG U+ / TONLY **MTTL-W01** strip | Metered 4-outlet smart strip (optional) |
| Onboard RGB LED | Status indicator |

---

## Wiring

| Signal | GPIO |
|---|---|
| Mic WS / SD / SCK | 15 / 17 / 16 |
| Amp BCLK / LRC / DIN | 5 / 7 / 8 |
| Relay IN1 / IN2 | 1 / 2 |
| IR TX / RX (defaults) | 4 / 6 |

- **MAX98357A:** VIN→5 V, GND→GND, leave `SD` and `GAIN` unconnected (≈ 9 dB).
- **Relay (isolated):** ESP32 3.3 V→`VCC`, external 5 V→`JD-VCC` (remove the jumper), common GND, GPIO1→IN1, GPIO2→IN2. Modules are assumed **active-low** (`RELAY_ACTIVE_LOW`).
- **IR pins are selectable from the web UI** from a safe list: `1, 2, 4, 6, 9–14, 18, 21, 38–42, 47` (strapping, USB, UART0, PSRAM, mic/amp and LED pins are excluded). Do not reuse the relay pins for IR.
- Drive the IR LED through a transistor for useful range.

---

## Repository layout

| File | Description |
|---|---|
| `Test_V5_1.ino` | Main sketch: audio, wake word, VAD, STT, matching, command engine, web routes, OTA, NTP, logging |
| `index_html.h` | Complete web UI (HTML/CSS/JS) as a PROGMEM string |
| `ir_module.h` | IR capture, storage and transmit + `/api/ir/*` routes |
| `strip_module.h` | MTTL-W01 TCP controller, telemetry parser + `/api/strip/*` routes |
| `pwa_assets.h` | PWA manifest, service worker and icons |
| `audio_clips.h` | Pre-rendered 16 kHz / 16-bit PCM voice replies |
| `partitions.csv` | Custom partition table (two 4 MB OTA slots, LittleFS, ESP-SR model partition) |

---

## Build & flash

**Requirements**
- Arduino IDE with the **ESP32 Arduino core 3.x** (the code uses `ledcAttach`, `esp_sntp.h`, ESP-SR headers).
- Libraries: `ArduinoJson` (v6), `WakeOnLan`, plus the core-bundled `WiFi`, `WebServer`, `LittleFS`, `HTTPClient`, `ArduinoOTA`, `Update`.
- A board with **PSRAM enabled** (OPI PSRAM on N16R8).

**Steps**
1. Put all files from this repo in one sketch folder (the `.ino` name must match the folder name).
2. Set **Tools → Partition Scheme → Custom** and use the provided `partitions.csv`:

   | Partition | Size | Use |
   |---|---|---|
   | `app0` / `app1` | 4 MB / ~4 MB | OTA application slots |
   | `spiffs` | 4 MB | LittleFS (commands, IR codes, settings) |
   | `model` | 4 MB | ESP-SR WakeNet models |

3. Flash the **ESP-SR model** into the `model` partition (WakeNet needs it; the sketch lists available models on the serial console if the chosen wake word is missing).
4. Edit the [configuration](#configuration) section, then upload.
5. Open the serial console at 115200 baud, note the IP address, and browse to `http://<ip>/`.

---

## Configuration

Edit the constants near the top of `Test_V5_1.ino`:

| Setting | Meaning |
|---|---|
| `ssid`, `password` | Wi-Fi credentials |
| `WIT_AI_TOKEN` | Wit.ai server access token |
| `WAKE_WORD` | `alexa`, `jarvis` or `hiesp` |
| `OTA_HOSTNAME`, `OTA_PASSWORD` | OTA identity (user is `admin` for `/update`) |
| `TZ_INFO` | POSIX time-zone string (default: Egypt `EET-2EEST,M4.5.5/0,M10.5.4/24`) |
| `SPK_VOLUME` | Reply volume 0–100 |
| `RETRY_MAX` | Automatic re-listen attempts after a failed command (0 disables) |
| `RELAY_ACTIVE_LOW` | `1` for the usual active-low relay boards |
| `VAD_*` | Silence length, no-speech timeout, pad, noise multiplier |
| `MIC_SHIFT`, `REC_TARGET_PEAK`, `REC_MAX_GAIN` | Capture gain/normalisation |
| `STANDBY_LED_MS` / `STANDBY_LED_BLUE` | Standby LED duration/brightness |

Microphone gain, VAD minimum level and “record what the device heard” are adjustable at runtime in **Maintenance → Microphone** and persisted in `/mic_cfg.json`.

> The WoL broadcast address defaults to `192.168.1.255` — change it in `setup()` if your subnet differs.

---

## Web UI guide

The UI has three top-level tabs. Swipe left/right to move between them — the tabs wrap around (swipe past *Maintenance* to land on *Commands*).

### Commands
- **Add/edit form:** name, category, spoken phrases (one per line), and ordered steps. Steps can be reordered with ▲▼ or by dragging the ⠿ handle.
- **Command list:** grouped by category (collapsible, reorderable, searchable). Each command shows its phrases, a numbered step list, and a copyable `/api/run?id=N` link. ▶ runs it immediately.
- **Last heard:** the recent transcriptions with ✔/✖ match status. Press ＋ to add a mis-heard phrase to a command.

### Strip (المشترك)
Live dashboard for the MTTL-W01: total watts and **total amps**, kWh, number of active outlets, per-outlet watts / **amps** / kWh / temperature, one-tap outlet toggle, master switch, rename outlets (stored in the browser), and a **Refresh** button. The ⚙ panel shows MAC/firmware and a **raw telemetry viewer** for diagnostics.

### Maintenance (الصيانة)
A scrollable sub-menu; **Log** opens by default.

| Section | Contents |
|---|---|
| **Log** | Live Serial Monitor with device timestamps, pause, clear, copy, download, level filter, text filter, and a text-command box to simulate speech |
| **Device** | IP, Wi-Fi signal, free RAM/PSRAM, uptime, wake word, device clock; PWA install; reboot |
| **IR** | Learn new signals, saved signals (send/rename/delete), TX/RX pin selection |
| **Microphone** | Wake-word gain, VAD minimum level, last-recording statistics |
| **Backup** | Export/import commands + IR codes as JSON |

The UI follows the system dark/light theme (toggle in the header) and is installable as a PWA (Chrome requires a secure origin or the `unsafely-treat-insecure-origin-as-secure` flag for plain HTTP).

---

## Commands, phrases and steps

Stored in `/commands.json` on LittleFS:

```json
[
  {
    "id": 1,
    "name": "PlayMode ON",
    "cat": "Automation",
    "phrases": ["start gaming", "ابدأ اللعب"],
    "steps": [
      { "type": "strip", "val": "1:on" },
      { "type": "delay", "val": "6.0" },
      { "type": "wol",   "val": "D8:5E:D3:C2:A1:F8" },
      { "type": "ir",    "val": "3" }
    ]
  }
]
```

| Step type | `val` format | Action |
|---|---|---|
| `http` | full URL | HTTP GET (3 s timeout; 2xx/3xx = success) |
| `wol` | MAC address | Send a Wake-on-LAN magic packet |
| `ir` | IR signal id | Transmit a learned signal |
| `relay` | `1:on`, `2:off`, `1:toggle` | Local relay |
| `strip` | `<outlet>:<on\|off\|toggle>`, outlet `0` = all | MTTL-W01 outlet |
| `delay` | seconds (0.1 – 3600) | Pause, then continue with the next step |

**Delays are non-blocking.** A `delay` step parks the remaining steps in a small job queue (4 slots) that `loop()` resumes later, so voice, web and strip traffic keep running. Timing uses `millis()`.

Each step and each resumed delay is logged with a timestamp, for example:

```
[06:26:03.412 PM] Step 1/4 [strip] 1:on
[06:26:03.455 PM] Delay 6.0s then continue from step 3
[06:26:09.470 PM] Delay finished -> resuming at step 3
```

---

## IR module

- **Capture:** the receiver pin is sampled by an edge interrupt storing microsecond timestamps; a 40 ms gap ends the frame. Up to 600 edges per signal; noise (< 5 pulses) is ignored and listening continues. Timeout: 15 s.
- **Storage:** signal metadata in `/ir.json`, raw pulse arrays in `/ir/<id>.bin`, pin choice in `/ir_cfg.json`.
- **Transmit:** LEDC PWM carrier (30–60 kHz, default 38 kHz, ≈ 33 % duty) with microsecond-accurate mark/space timing and 1–5 repeats.
- **Workflow:** *Maintenance → IR → Start learning* → press the remote button → *Try* → name and save → use it in a command as an `IR` step.
- Learned signals are included in backups and re-linked to commands automatically on import.

---

## Power-strip module (MTTL-W01)

The ESP32 acts as the **controller**: the strip connects *to the ESP32* over TCP port **10086** (the same role the Windows “MTTL Control” app normally plays).

**One-time setup:** in the MTTL Control app go to *Device setup* and enter the **ESP32’s IP address** (shown in the Strip tab) with port 10086 instead of the PC’s IP.

Protocol notes (plain UTF-8 lines ending in `\r\n`):

| Direction | Message |
|---|---|
| Strip → ESP32 | `up:bootinfo:<model>;<mac>;<mac>;<fw>;connect` (handshake) |
| ESP32 → Strip | `up:getinfo:all` (polled every 10 s) |
| Strip → ESP32 | `up:getinfo:1:<rec>:2:<rec>:3:<rec>:4:<rec>` |
| ESP32 → Strip | `up:onoff:<0-4>:<on\|off>` (0 = master) |
| ESP32 → Strip | `up:power_report:1:vol` (line voltage, polled with telemetry) |

Each outlet record has 12 `;`-separated fields; the parser uses: `F1` relay state, `F3/F4` over-voltage/over-temperature OK flags, `F5` power in mW, `F6` cumulative energy in Wh (hex), `F10` event flags (hex), `F11` temperature °C.

Behaviour:
- New connections are accepted only after a valid `bootinfo`; a newer valid connection replaces an older one.
- A session with no data for 45 s is dropped.
- **Current (A)** is computed as `W ÷ V`, using the reported line voltage (falls back to 220 V and is marked `≈` if unavailable). It is derived from *real* power, so it under-reads on loads with a low power factor.
- The Strip tab’s *raw telemetry* viewer prints every field per outlet — useful for checking whether a given outlet has a metering channel.

---

## Time & timestamps

- On Wi-Fi connect the device starts SNTP (`pool.ntp.org`, `time.google.com`, `time.cloudflare.com`) with the configured `TZ_INFO`. No blocking wait.
- Every log line is prefixed by the **device** with `[hh:mm:ss.mmm AM/PM]` — both on the USB serial port and in the web Serial Monitor. Previously the browser stamped lines on arrival, so bursts looked simultaneous.
- Until the clock is set, lines use device uptime: `[+hh:mm:ss.mmm]`.
- **Fallback:** if NTP is unreachable (e.g., no internet), the web page pushes the phone’s time to the device (`POST /api/time`).
- Change `TZ_INFO` for other regions (e.g. `UTC0`, `CET-1CEST,M3.5.0,M10.5.0/3`).

---

## HTTP API reference

All endpoints are served on port 80. POST bodies are `application/x-www-form-urlencoded` unless noted.

**Commands & voice**

| Method | Path | Notes |
|---|---|---|
| GET | `/api/commands` | All commands (JSON) |
| POST | `/api/save` | JSON body `{id, name, cat, phrases[], steps[{type,val}]}`; `id: 0` creates |
| POST | `/api/delete` | `id` |
| GET/POST | `/api/run?id=N` | Run a command (works from shortcut apps) |
| POST | `/api/clear` | Delete all commands |
| POST | `/api/say` | `text` — simulate a spoken phrase |
| GET | `/api/heard` · POST `/api/heard/clear` | Recent transcriptions |
| GET/POST | `/api/heard/cfg` | Enable/disable transcription logging (`on`) |
| GET/POST | `/api/mic` | `g` (wake gain 2–128), `v` (VAD min level 2–60) |

**Device**

| Method | Path | Notes |
|---|---|---|
| GET | `/api/info` | IP, RSSI, heap, PSRAM, uptime, wake word, `sync`, `now` |
| GET/POST | `/api/time` | Read device time / set it with `ts` (epoch seconds; ignored if already synced unless `force=1`) |
| GET | `/api/log?since=N` | Incremental log (next offset in `X-Next` header) |
| POST | `/api/log/clear` | Clear log |
| POST | `/api/reboot` | Restart |
| GET/POST | `/api/relay?n=1&s=on\|off\|toggle` | Relay control / state |
| GET/POST | `/update` | Browser OTA (basic auth `admin` / `OTA_PASSWORD`) |

**IR**

| Method | Path | Notes |
|---|---|---|
| GET/POST | `/api/ir/cfg` | TX/RX pins |
| POST | `/api/ir/learn` | Start capture |
| GET | `/api/ir/status` | `idle / waiting / captured / timeout` |
| GET | `/api/ir/list` | Saved signals |
| POST | `/api/ir/save` | `id=0` stores the captured signal; `id>0` edits name/kHz/repeat |
| POST | `/api/ir/send` | `id`, or `pending=1&khz=&rep=` to test before saving |
| POST | `/api/ir/delete` | `id` |
| GET | `/api/ir/raw?id=N` | Raw pulses (comma-separated) |
| POST | `/api/ir/import?name=&khz=&rep=` | Body = pulses (text/plain) |

**Power strip**

| Method | Path | Notes |
|---|---|---|
| GET | `/api/strip/status` | Connection, per-outlet W / Wh / °C / state, line voltage `v` |
| POST | `/api/strip/set` | `n` (0–4), `s` (`on/off/toggle`) |
| POST | `/api/strip/refresh` | Request fresh telemetry |
| GET | `/api/strip/raw` | Last raw telemetry frame |

**PWA:** `/manifest.webmanifest`, `/sw.js`, `/icon-192.png`, `/icon-512.png`.

---

## Updating firmware (OTA)

- **Arduino IDE:** *Tools → Port → Network ports* (`voice-assistant`), password = `OTA_PASSWORD`.
- **Browser:** open `http://<ip>/update`, sign in as `admin`, upload the compiled `.bin`. The LED turns orange during the update and the device reboots when done.

---

## Troubleshooting

| Symptom | Things to check |
|---|---|
| Wake word never triggers | Raise *Microphone → wake gain*; confirm the `model` partition was flashed; check the serial output for `Using model:` |
| Cuts off the end of sentences / misses quiet speech | Lower the VAD minimum level; adjust `VAD_SILENCE_MS` |
| “Didn’t understand” on correct speech | Open *Last heard* — add the actual transcription as a phrase |
| Strip shows *Offline* | Re-enter the ESP32’s IP in MTTL Control → Device setup; the strip must reach port 10086 |
| Strip outlet reads 0 W with a load | Open the raw telemetry viewer; if F5/F6/F7 are all `0` for that outlet it has no metering channel (or the load is below the meter’s resolution) |
| Amps shown with `≈` | Line-voltage reply wasn’t received; check the log for `Strip volt: unexpected reply` |
| Log times look wrong | Open *Maintenance → Device* and check the `Time` chip; ⚠ means the clock isn’t synced |
| IR doesn’t control a device | Use a transistor on the LED, check the carrier (try 36/40/56 kHz), and increase repeats |
| Can’t install as PWA over HTTP | Chrome flag `chrome://flags/#unsafely-treat-insecure-origin-as-secure` with `http://<ip>` |

---

## Security notes

- **Do not commit real secrets.** Replace `ssid`, `password`, `WIT_AI_TOKEN` and `OTA_PASSWORD` with placeholders (or move them to an untracked header) before publishing.
- The web UI and `/api/*` endpoints have **no authentication** — only `/update` is password-protected. Run the device on a trusted LAN and do not expose port 80 to the internet.
- The default `OTA_PASSWORD` is a placeholder; change it.
- The Wit.ai audio upload uses HTTPS with certificate verification disabled (`setInsecure()`); pin a certificate if that matters for your threat model.

---

## Credits

- [ESP-SR / WakeNet](https://github.com/espressif/esp-sr) — wake-word engine
- [ArduinoJson](https://arduinojson.org/), [WakeOnLan](https://github.com/a7md0/WakeOnLan)
- MTTL-W01 protocol notes from the community MTTL Control app and strip’s own bootinfo/telemetry traffic

## License

Add a license of your choice (e.g. MIT) as `LICENSE`.
