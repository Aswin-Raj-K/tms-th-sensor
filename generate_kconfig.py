#!/usr/bin/env python3
import json
import os

def generate_kconfig(devices_file='devices.json', output_file='main/Kconfig.projbuild'):
    with open(devices_file, 'r') as f:
        data = json.load(f)
    
    devices = data['devices']
    default_sensor = data.get('default_sensor', 5)
    
    max_sensor_id = max(d['id'] for d in devices)
    
    kconfig_content = []
    kconfig_content.append('menu "TH Sensor Device Configuration"')
    kconfig_content.append('')
    kconfig_content.append('    choice SENSOR_DEVICE')
    kconfig_content.append('        prompt "Select Sensor Device"')
    kconfig_content.append(f'        default SENSOR_{default_sensor}')
    kconfig_content.append('        help')
    kconfig_content.append('            Select which sensor device to build for.')
    kconfig_content.append(f'            This can also be set via SENSOR_ID environment variable (1-{max_sensor_id}).')
    kconfig_content.append('')
    
    for device in devices:
        kconfig_content.append(f'        config SENSOR_{device["id"]}')
        kconfig_content.append(f'            bool "{device["name"]} - {device["description"]}"')
        kconfig_content.append(f'            help')
        kconfig_content.append(f'                Device ID: {device["device_id"]}')
        kconfig_content.append(f'                Device Name: {device["name"]}')
        kconfig_content.append(f'                Location: {device["description"]}')
    
    kconfig_content.append('    endchoice')
    kconfig_content.append('')
    
    kconfig_content.append('    config DEVICE_ID')
    kconfig_content.append('        int')
    for device in devices:
        kconfig_content.append(f'        default {device["device_id"]} if SENSOR_{device["id"]}')
    kconfig_content.append('')
    
    kconfig_content.append('    config DEVICE_DESCRIPTION')
    kconfig_content.append('        string')
    for device in devices:
        kconfig_content.append(f'        default "{device["description"]}" if SENSOR_{device["id"]}')
    kconfig_content.append('')
    
    kconfig_content.append('    config DEVICE_NAME')
    kconfig_content.append('        string')
    for device in devices:
        kconfig_content.append(f'        default "{device["name"]}" if SENSOR_{device["id"]}')
    kconfig_content.append('')
    
    kconfig_content.append('endmenu')
    kconfig_content.append('')
    
    with open(output_file, 'w') as f:
        f.write('\n'.join(kconfig_content))
    
    print(f"✓ Generated {output_file} from {devices_file}")
    print(f"  - {len(devices)} devices configured")
    print(f"  - Default sensor: {default_sensor}")

if __name__ == '__main__':
    script_dir = os.path.dirname(os.path.abspath(__file__))
    os.chdir(script_dir)
    generate_kconfig()
