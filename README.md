# TH_Sensor_V1

ESP32 firmware project for a temperature/humidity sensor node that:
- reads SHT3x data over I2C,
- buffers samples locally,
- syncs time over SNTP,
- connects to Wi-Fi,
- communicates with a backend over TLS using JSON,
- optionally displays live status on an SSD1306 OLED.

## Project Summary

This project is built with ESP-IDF (`idf` dependency lock indicates `5.3.0`) and targets `esp32`.

At runtime, `app_main()` initializes storage and peripherals, joins Wi-Fi, waits for time sync, then starts HTTPS client tasks. The device periodically sends sensor data and supports status/config updates exchanged through JSON messages.

## Features

- SHT3x temperature/humidity acquisition
- Local circular buffer for readings
- Optional data averaging before upload
- Persistent configuration in NVS
- Wi-Fi station support (WPA2 personal and enterprise logic exists)
- TLS client transport to backend server
- Alarm trigger checks for threshold breaches
- Optional OLED screen updates

## Requirements

## Hardware
- ESP32 board
- SHT3x sensor on I2C
- Optional SSD1306 I2C OLED
- USB cable for flashing/monitoring

## Software
- ESP-IDF `v5.3.x` (project lock file indicates `5.3.0`)
- CMake (bundled with ESP-IDF toolchain)
- Python environment configured by ESP-IDF tools

## Build and Flash

From project root:

```bash
idf.py set-target esp32
idf.py build
idf.py -p <PORT> flash monitor
```

Example:

```bash
idf.py -p COM5 flash monitor
```

## Runtime Flow

1. `main/main.c` initializes NVS and event loop.
2. I2C and sensor/OLED are initialized (unless debug path disables hardware init).
3. Storage defaults and saved config are loaded from NVS.
4. Wi-Fi manager connects and invokes `wifi_connected()` callback.
5. SNTP initializes and waits for valid system time.
6. HTTPS client starts data and listener tasks for backend communication.

## Configuration Notes

Current code keeps network and endpoint configuration in headers:

- Wi-Fi credentials/type: `components/wifi_manager/wifi_manager.h`
- Server host/port and certificate: `components/https_client/https_client.h`
- Device identity/name: `components/device/device.h`

Recommended improvement: move credentials and environment-specific values into `sdkconfig`/menuconfig or private config headers excluded from version control.

## Source File Details

### Root files

- `CMakeLists.txt`  
  Top-level ESP-IDF project declaration (`project(TH_Sensor_V1)`).
- `dependencies.lock`  
  Dependency lock metadata (includes ESP-IDF version and target).
- `sdkconfig` / `sdkconfig.old` / `sdkconfig.ci`  
  Build-time configuration snapshots.
- `pytest_hello_world.py`  
  Default ESP-IDF sample test file (not aligned with this firmware behavior).
- `README.md`  
  Project documentation.

### `main/`

- `main/CMakeLists.txt`  
  Registers `main.c` and component dependencies (`wifi_manager`, `https_client`, `esp_time`, etc.).
- `main/main.c`  
  Application entrypoint (`app_main`), startup sequence, Wi-Fi callback, SNTP start, and HTTPS client startup.

### `components/wifi_manager/`

- `wifi_manager.h`  
  Wi-Fi mode/credentials macros and API declarations.
- `wifi_manager.c`  
  Event handlers for Wi-Fi/IP events, reconnect logic, and station configuration for WPA2 personal/enterprise.
- `CMakeLists.txt`  
  Component registration and ESP-IDF dependency declaration.

### `components/https_client/`

- `https_client.h`  
  TLS/server config, certificate, request/response constants, JSON key constants, and public APIs.
- `https_client.c`  
  Core backend communication logic: TLS connect/write/read, register/save/status/data requests, alarm handling, screen updates, and synchronization primitives.
- `CMakeLists.txt`  
  Component registration and required component list (`esp-tls`, `json`, sensor/display/storage components).

### `components/data_storage/`

- `data_storage.h`  
  Storage keys, default values, state constants, and storage API.
- `data_storage.c`  
  NVS-backed read/write of runtime settings (hold time, buffer size, averaging mode, thresholds, state).
- `CMakeLists.txt`  
  Component registration with `nvs_flash` dependency.

### `components/device/`

- `device.h`  
  Device metadata (`DEVICE_ID`, name/description), JSON field constants, sample struct.
- `device.c`  
  Circular buffer management for readings, average calculation, latest sample retrieval, JSON packing of buffered data.
- `CMakeLists.txt`  
  Component registration and dependencies (`data_storage`, `json`).

### `components/esp_time/`

- `esp_time.h`  
  SNTP/time API declarations.
- `esp_time.c`  
  SNTP initialization, time sync wait/retry behavior, timezone setup, and helper to fetch epoch time.
- `CMakeLists.txt`  
  Component registration.

### `components/i2c/`

- `i2c.h`  
  I2C pin/speed definitions and bus helper APIs.
- `i2c.c`  
  Initializes I2C master bus, adds/removes devices from bus.
- `CMakeLists.txt`  
  Component registration.

### `components/sht3x/`

- `sht3x.h`  
  Sensor constants and read/init APIs.
- `sht3x.c`  
  SHT3x measurement command flow, CRC check, retry/reset handling, converted temperature/humidity output with calibration offsets.
- `CMakeLists.txt`  
  Component registration.

### `components/ssd1306/`

- `ssd1306.h`  
  SSD1306 command constants, display buffer structures, and rendering APIs.
- `ssd1306.c`  
  OLED rendering and I2C display operations.
- `font8x8_basic.h`  
  Bitmap font definitions used by OLED text rendering.
- `CMakeLists.txt`  
  Component registration (`driver`, `i2c` requirements).

## Data/Message Behavior (High-Level)

The device uses JSON message fields such as:
- request/response IDs,
- device metadata,
- batched sensor readings (`temp`, `hum`, `time`),
- runtime config info (average mode, hold time, buffer size),
- alarm trigger payloads.

Protocol constants are primarily in:
- `components/https_client/https_client.h`
- `components/device/device.h`

## Troubleshooting

- Build fails due to environment:
  - Ensure ESP-IDF tools are exported in the shell.
  - Confirm `idf.py --version` works.
- Flash/monitor issues:
  - Verify COM port and board connection.
  - Try lower baud in monitor/flash options if unstable.
- No sensor data:
  - Verify I2C wiring matches pin definitions in `components/i2c/i2c.h` (`SDA_IO_NUM`, `SCL_IO_NUM`).
  - Check sensor power and address (`0x44` for SHT3x in current code).
- Cannot connect to backend:
  - Verify `SERVER_HOST` and `SERVER_PORT`.
  - Confirm certificate/server settings are valid for your environment.

## Next Cleanup Suggestions

- Move secrets and per-site constants out of tracked headers.
- Replace default `pytest_hello_world.py` with project-specific integration tests.
- Add architecture diagram and protocol examples for backend integration.
