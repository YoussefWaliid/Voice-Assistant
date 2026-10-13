# Changelog — Voice Hub (ESP32-S3)

Base: `Test_V4_3.ino` · Date: 2026-10-11

> Status: none of these changes have been compiled or hardware-verified by the author of this log, except where noted. The strip connection was confirmed working by the user after fix 2.

## Added

### MTTL-W01 power strip control (`strip_module.h`, new file)
- ESP32 now acts as the strip's **controller**: TCP server on port **10086**; the strip connects outbound to it.
- Session handling: a new connection is held as *pending* until a valid `up:bootinfo:` arrives, then replaces the old session (one session per strip).
- Polls `up:getinfo:all` every 10 s; also after each command (~700 ms) for confirmation.
- Telemetry parsed per outlet: relay state, power (W), cumulative energy (Wh), temperature, protection flags.
- Dead-session timeout: 45 s without data.
- New action type **`strip`**, value `<outlet>:<state>`: outlet `0` = all, `1–4` = single; state `on` / `off` / `toggle`.
- New endpoints: `GET /api/strip/status`, `POST /api/strip/set` (`n`, `s`), `POST /api/strip/refresh`.

### Delay step with non-blocking scheduler
- New action type **`delay`** (value in seconds, e.g. `5` or `2.5`, max 3600).
- Remaining steps run later from `loop()` via `jobPoll()`, so voice, web and strip polling stay responsive.
- Up to 4 delayed jobs at once. A trailing delay (no steps after it) is ignored.

### Microphone tuning
- Runtime-adjustable **wake-word gain** (`micGain`, default 16 ≈ old `MIC_SHIFT 12`) and **speech floor** (`vadMinLevel`, default 8), saved to `/mic_cfg.json`.
- Continuous **idle noise estimate** (`noiseIdle`), used to seed the VAD instead of the recording's own quietest moment.
- **High-pass filter (120 Hz)** on both the wake-word path and the recording (removes DC offset and rumble).
- New endpoints: `GET /api/mic`, `POST /api/mic` (`g` 2–128, `v` 2–60).

### Recognition
- **Fuzzy matching** as a third pass after the exact passes: character-level edit distance per word (words ≤ 3 chars must match exactly; 4–6 chars allow 1 edit; 7+ allow 2).
- **Heard log**: last 10 transcriptions with match status, `GET /api/heard`.

### Web UI (`index_html.h`)
- New **"المشترك"** tab: connection status, MAC, firmware, per-outlet cards (state, W, kWh, °C, toggle button), all-on / all-off, controller IP hint for provisioning.
- Step editor: new types **مشترك** (outlet + action selects) and **تأخير** (seconds field).
- Commands tab: **"آخر ما سمعه الجهاز"** card with a ＋ button that appends the heard text to the phrases box.
- Maintenance tab: **Microphone** card (gain, speech floor, last-recording stats).

## Changed
- `RECORD_TIME_SECONDS`: 4 → 6.
- `REC_MAX_GAIN`: 200 → 400.
- `VAD_MIN_LEVEL` (fixed 40) replaced by runtime `vadMinLevel` (default 8). The old value was above normal speech level at ~1 m, which is why speaking normally from a distance was not picked up.
- Command save validation now accepts `strip` (validated format) and `delay` (positive number, clamped to 3600 s).

## Fixed
- **Strip connect/disconnect loop**: `now` was captured at the top of `stripPoll()`, then `stripLastRx` was set later in the same call, so `now - stripLastRx` underflowed (unsigned) and triggered an instant "timeout". Now uses a fresh `millis()` and signed comparisons.
- **Visual glitch in the step editor**: `display:flex` on the new fields overrode the `hidden` attribute. Added `[hidden]{display:none!important}`.

## Files changed
| File | Change |
|---|---|
| `Test_V4_3.ino` | strip/delay actions, job scheduler, mic processing, fuzzy match, heard log, new routes |
| `strip_module.h` | new |
| `index_html.h` | new tab, step types, heard card, mic card, CSS fix |
| `ir_module.h`, `audio_clips.h`, `partitions.csv` | unchanged |

## Setup notes
- Re-provision the strip once from MTTL Control (*Device setup*) using the **ESP32's IP** as the controller address; reserve that IP on the router.
- The strip connects to one controller only, so the desktop app no longer receives data while it points at the ESP32.
- Master outlet `0` appears in the app's protocol console, but the protocol guide only documents outlets 1–4; test it on the device.

## Known limitations
- While recording/transcribing, `loop()` is blocked for a few seconds, so strip data is not read during that time (the connection stays open).
- Pending delayed steps are lost if the ESP32 resets.
- The strip protocol is plaintext; keep it on the local network.
- Digital gain does not improve signal-to-noise ratio; beyond roughly 2–3 m, hardware changes (mic placement, noise suppression) are needed.
- The Wit.ai token is hard-coded in the sketch. Remove it before publishing the code.

## Discussed, not implemented
- AI question answering (LLM + TTS) — paused at the user's request.
- ESP-SR AFE (noise suppression / AGC / echo cancellation), alternative STT providers, pre-roll buffer, streaming audio upload.
