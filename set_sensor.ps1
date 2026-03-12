#!/usr/bin/env pwsh
# PowerShell script to set sensor device and build

param(
    [Parameter(Mandatory=$false)]
    [int]$SensorId
)

# Load devices from JSON file
$devicesJsonPath = Join-Path $PSScriptRoot "devices.json"
if (-not (Test-Path $devicesJsonPath)) {
    Write-Host "Error: devices.json not found at $devicesJsonPath" -ForegroundColor Red
    exit 1
}

$devicesData = Get-Content $devicesJsonPath | ConvertFrom-Json
$sensors = @{}
$maxSensorId = 0

foreach ($device in $devicesData.devices) {
    $sensors[$device.id] = @{
        Name = $device.name
        Description = $device.description
        Id = $device.device_id
    }
    if ($device.id -gt $maxSensorId) {
        $maxSensorId = $device.id
    }
}

if (-not $SensorId) {
    Write-Host "`nAvailable Sensors:" -ForegroundColor Cyan
    Write-Host "==================" -ForegroundColor Cyan
    foreach ($key in 1..$maxSensorId) {
        $s = $sensors[$key]
        Write-Host "$key. $($s.Name) - $($s.Description) (ID: $($s.Id))"
    }
    Write-Host ""
    $SensorId = Read-Host "Select sensor (1-$maxSensorId)"
    $SensorId = [int]$SensorId
}

if ($SensorId -lt 1 -or $SensorId -gt $maxSensorId) {
    Write-Host "Error: Invalid sensor ID. Must be 1-$maxSensorId." -ForegroundColor Red
    exit 1
}

$selected = $sensors[$SensorId]
Write-Host "`nSelected: Sensor $SensorId - $($selected.Name)" -ForegroundColor Green
Write-Host "Description: $($selected.Description)" -ForegroundColor Green
Write-Host "Device ID: $($selected.Id)" -ForegroundColor Green
Write-Host ""

# Set environment variable for this session
$env:SENSOR_ID = $SensorId

# Update sdkconfig with the selected sensor
Write-Host "Configuring build for Sensor $SensorId..." -ForegroundColor Yellow

# Run menuconfig in non-interactive mode to apply defaults
idf.py set-target esp32

# Directly modify sdkconfig to set the sensor choice
$sdkconfigPath = "sdkconfig"
if (Test-Path $sdkconfigPath) {
    $content = Get-Content $sdkconfigPath
    $newContent = @()
    $inSensorSection = $false
    
    foreach ($line in $content) {
        # Remove old sensor selections
        if ($line -match "^CONFIG_SENSOR_\d+=") {
            continue
        }
        $newContent += $line
    }
    
    # Add new sensor selection
    $newContent += "CONFIG_SENSOR_$SensorId=y"
    
    Set-Content -Path $sdkconfigPath -Value $newContent
}

Write-Host "`nBuilding firmware..." -ForegroundColor Yellow
idf.py build

if ($LASTEXITCODE -eq 0) {
    Write-Host "`nBuild successful for Sensor $SensorId ($($selected.Name))!" -ForegroundColor Green
    Write-Host "To flash: idf.py -p COM<X> flash monitor" -ForegroundColor Cyan
} else {
    Write-Host "`nBuild failed!" -ForegroundColor Red
    exit $LASTEXITCODE
}
