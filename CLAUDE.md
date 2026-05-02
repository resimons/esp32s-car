# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Project Overview

ESP32-based RC car firmware using PlatformIO + Arduino framework. Target board: `nodemcu-32s`. The car has 4 motors (L/R front/back), controlled via GPIO PWM signals. It connects to WiFi, receives drive commands over MQTT with TLS, and publishes status.

## Build & Flash Commands

```bash
# Build
pio run

# Upload to device
pio run --target upload

# Monitor serial output (115200 baud)
pio device monitor

# Build + upload + monitor in one step
pio run --target upload && pio device monitor
```

There are no automated tests; verification is done via serial monitor output.

## Configuration

`src/config.h` is gitignored. Copy `src/config-example.h` to `src/config.h` and fill in:
- WiFi SSID/password
- MQTT broker host, port, and topic names (`mqttSensorTopic`, `mqttAliveTopic`, `mqttCommandTopic`, `mqttStatusTopic`)
- TLS certificates (root CA, client cert, private key)

## Architecture

**`src/main.cpp`** — entry point. `setup()` initialises WiFi, MQTT, and engine; publishes an `iamalive` message. `loop()` calls `mqtt_loop()` and `engine_loop()` every iteration.

**`src/engine.h/.cpp`** — motor control. Each wheel's state (GPIO pins, logical speed 0–255, direction) is tracked in a `WheelState` struct held in a static array. `applyWheel()` is the single PWM output point; it applies a hardware minimum of 80 to the PWM value but does **not** write back to `WheelState.speed`, so the logical speed always tracks the intended value. The public API has four layers:

- **Per-wheel**: `wheelLeftBackForward(speed)`, `wheelLeftBackBackward(speed)`, `wheelLeftBackReverse()`, `wheelLeftBackSpeed()`, `wheelLeftBackIsForward()` — and equivalents for the other three wheels.
- **Turning**: `pivotLeft(speed)` / `pivotRight(speed)` (spin in place); `arcLeft(innerSpeed, outerSpeed)` / `arcRight(innerSpeed, outerSpeed)` (curve while moving).
- **All-wheel**: `moveForward(speed, time)`, `moveBackward(speed, time)`, `reverseAll()`, `adjustSpeed(delta)`, `stop()`.
- **Non-blocking timing**: `scheduleStop(duration)` arms an auto-stop after `duration` ms; `rampTo(targetSpeed, stepMs)` steps every wheel's speed toward `targetSpeed` by 1 unit every `stepMs` ms. Both are driven by `engine_loop()`.

Any direction-changing command (`moveForward`, `pivotLeft`, `arcLeft`, `stop`, `adjustSpeed`, `reverseAll`) cancels an active ramp. `rampTo` does not cancel a scheduled stop — the two are orthogonal.

**`src/wifiCommunication.h/.cpp`** — WiFi management with event callbacks for connect/disconnect/GotIP. Auto-reconnects on disconnect. `wifiIsDisabled` global guards all network calls.

**`src/mqtt.h/.cpp`** — TLS MQTT via `WiFiClientSecure` + `MQTTClient`. Subscribes to `mqttCommandTopic` on connect (and re-subscribes on reconnect). Reconnects every 5 s when disconnected. `mqtt_loop()` runs the MQTT client loop and advances any active command routine. Incoming JSON is dispatched via `executeCommand()` (internal, routine-safe) or `handleCommand()` (public, cancels any running routine first). Status telemetry is published to `mqttStatusTopic`.

**`src/ir-commands.h`** — IR command code constants (FORWARD=96, BACKWARD=97, RIGHT=98, LEFT=101).

**`src/led-display.h/.cpp`** — TM1637 LED segment display utilities (degrees, spinner).

## GPIO Pin Mapping

| Motor | Forward pin | Backward pin |
|---|---|---|
| Left Back | GPIO 12 | GPIO 13 |
| Left Front | GPIO 14 | GPIO 15 |
| Right Front | GPIO 25 | GPIO 26 |
| Right Back | GPIO 32 | GPIO 33 |

## MQTT Command Protocol

Publish JSON to `mqttCommandTopic`. All fields except `command` are optional.

```json
{"device": "MCUDEVICE-ABCDEF", "command": "pivotLeft", "speed": 150, "duration": 500}
```

| Field | Type | Description |
|---|---|---|
| `command` | string | See table below |
| `speed` | int (0–255) | PWM speed (or delta for `adjustSpeed`, target for `rampTo`) |
| `duration` | int (ms) | Auto-stop after this many ms (step interval for `rampTo`) |
| `device` | string | Device ID to target; omit to broadcast |

| Command | Behaviour |
|---|---|
| `forward` | All wheels forward at `speed` |
| `backward` | All wheels backward at `speed` |
| `pivotLeft` | Left wheels backward, right wheels forward — spins in place |
| `pivotRight` | Left wheels forward, right wheels backward — spins in place |
| `arcLeft` | Left at `speed/2`, right at `speed` — curves left while moving |
| `arcRight` | Left at `speed`, right at `speed/2` — curves right while moving |
| `adjustSpeed` | Add `speed` (may be negative) to every wheel's current speed |
| `rampTo` | Non-blocking ramp all wheels to `speed`; `duration` = ms per step |
| `reverseAll` | Flip all wheels, keep current speeds |
| `status` | Publish current wheel speeds and directions to `mqttStatusTopic` |
| `stop` | Stop all wheels immediately |
| `routine` | Execute a sequence of steps (see below) |

### Routine command

```json
{"command": "routine", "steps": [
  {"command": "forward",   "speed": 150, "duration": 2000},
  {"command": "pivotLeft", "speed": 150, "duration": 600},
  {"command": "forward",   "speed": 150, "duration": 1500},
  {"command": "stop",      "speed": 0,   "duration": 0}
]}
```

Each step runs for `duration` ms before the next step starts. Maximum 10 steps. Any non-routine command cancels a running routine. `duration` in routine steps has its normal meaning for all commands except `rampTo`, where it is the step interval.

### Status response

Published to `mqttStatusTopic`:

```json
{"device": "MCUDEVICE-ABCDEF",
 "leftBack":   {"speed": 125, "forward": true},
 "leftFront":  {"speed": 125, "forward": true},
 "rightFront": {"speed": 125, "forward": true},
 "rightBack":  {"speed": 125, "forward": true}}
```

## Key Libraries (platformio.ini)

- `256dpi/MQTT` — MQTT client
- `bblanchon/ArduinoJson` — JSON command parsing and status serialisation
- `z3t0/IRremote` — IR receiver
- `adafruit/Adafruit GFX Library` — display
- `smougenot/TM1637` — 7-segment display
- `marcoschwartz/LiquidCrystal_I2C` — I2C LCD
