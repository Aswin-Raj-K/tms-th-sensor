#!/bin/bash
# Bash script to set sensor device and build

sensors=(
    "1:TH_GR:Gowning room 819 A:4000"
    "2:TH_DR:Deposition room 819C:4001"
    "3:TH_ER:Etch room 819D:4002"
    "4:TH_EBLR:EBL room 819E:4003"
    "5:TH_YR:Yellow room 819F:4004"
)

if [ -z "$1" ]; then
    echo -e "\n\033[1;36mAvailable Sensors:\033[0m"
    echo "=================="
    for sensor in "${sensors[@]}"; do
        IFS=':' read -r id name desc devid <<< "$sensor"
        echo "$id. $name - $desc (ID: $devid)"
    done
    echo ""
    read -p "Select sensor (1-5): " SENSOR_ID
else
    SENSOR_ID=$1
fi

if [ "$SENSOR_ID" -lt 1 ] || [ "$SENSOR_ID" -gt 5 ]; then
    echo -e "\033[1;31mError: Invalid sensor ID. Must be 1-5.\033[0m"
    exit 1
fi

# Get sensor info
sensor_info="${sensors[$SENSOR_ID-1]}"
IFS=':' read -r id name desc devid <<< "$sensor_info"

echo -e "\n\033[1;32mSelected: Sensor $SENSOR_ID - $name\033[0m"
echo -e "\033[1;32mDescription: $desc\033[0m"
echo -e "\033[1;32mDevice ID: $devid\033[0m"
echo ""

# Set environment variable
export SENSOR_ID=$SENSOR_ID

# Configure build
echo -e "\033[1;33mConfiguring build for Sensor $SENSOR_ID...\033[0m"

# Run set-target
idf.py set-target esp32

# Modify sdkconfig
if [ -f "sdkconfig" ]; then
    # Remove old sensor selections
    sed -i '/^CONFIG_SENSOR_[1-5]=/d' sdkconfig
    # Add new selection
    echo "CONFIG_SENSOR_$SENSOR_ID=y" >> sdkconfig
fi

# Build
echo -e "\n\033[1;33mBuilding firmware...\033[0m"
idf.py build

if [ $? -eq 0 ]; then
    echo -e "\n\033[1;32mBuild successful for Sensor $SENSOR_ID ($name)!\033[0m"
    echo -e "\033[1;36mTo flash: idf.py -p /dev/ttyUSB0 flash monitor\033[0m"
else
    echo -e "\n\033[1;31mBuild failed!\033[0m"
    exit 1
fi
