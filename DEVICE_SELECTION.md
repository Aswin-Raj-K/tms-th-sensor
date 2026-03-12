# Device Selection Guide

This project supports building firmware for 5 different TH sensor devices. You can select which device to build for using environment variables or helper scripts.

## Available Devices

| Sensor | Device ID | Name    | Description              |
|--------|-----------|---------|--------------------------|
| 1      | 4000      | TH_GR   | Gowning room 819 A       |
| 2      | 4001      | TH_DR   | Deposition room 819C     |
| 3      | 4002      | TH_ER   | Etch room 819D           |
| 4      | 4003      | TH_EBLR | EBL room 819E            |
| 5      | 4004      | TH_YR   | Yellow room 819F         |

## Method 1: Using Helper Scripts (Easiest)

### Windows (PowerShell)
```powershell
.\set_sensor.ps1
```
Or specify sensor directly:
```powershell
.\set_sensor.ps1 -SensorId 3
```

### Linux/Mac (Bash)
```bash
chmod +x set_sensor.sh
./set_sensor.sh
```
Or specify sensor directly:
```bash
./set_sensor.sh 3
```

The scripts will:
1. Prompt you to select a sensor (if not specified)
2. Configure the build automatically
3. Build the firmware
4. Show flash instructions

## Method 2: Using idf.py menuconfig

1. Open the configuration menu:
```bash
idf.py menuconfig
```

2. Navigate to: **TH Sensor Device Configuration**

3. Select your desired sensor from the list

4. Save and exit (press `S` then `Q`)

5. Build normally:
```bash
idf.py build
```

## Method 3: Manual Environment Variable

Set the `SENSOR_ID` environment variable before building:

### Windows (PowerShell)
```powershell
$env:SENSOR_ID = 3
idf.py build
```

### Windows (CMD)
```cmd
set SENSOR_ID=3
idf.py build
```

### Linux/Mac
```bash
export SENSOR_ID=3
idf.py build
```

## Method 4: Direct sdkconfig Edit

Edit `sdkconfig` file and add:
```
CONFIG_SENSOR_3=y
```
(Replace `3` with your desired sensor number 1-5)

Then build:
```bash
idf.py build
```

## Flashing

After building, flash to your device:
```bash
idf.py -p COM3 flash monitor    # Windows
idf.py -p /dev/ttyUSB0 flash monitor    # Linux/Mac
```

## Verification

The device configuration is compiled into the firmware. Check the logs during boot to verify the correct device ID and name are being used.
