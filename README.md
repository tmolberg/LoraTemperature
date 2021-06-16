# LoraTemperature

This project was created due to my inability of going for a swim in too cold waters... 
Its also been used for mailbox notification (yes the physical box)

Its based on a mishmash of everything I thought I could get working.

### Prerequisites - Software
- InfluxDB
- Grafana
- Mqtt Broker (Optional if you want this in your home automation setup)

### Prerequisites - Hardware
- Wisen WhisperNode
- DS18B20 temp probes
- Raspberry Pi 1/2/3/4/Z

### To be done...
#### **Python script**
- Implement better error handling on serial to influx converter
- Abstract serial to influx converter
    - So that other people can more easily use the scripts...

#### **Arduino Code**
- Convert ino files to vscode/platformio based files... 
    - Arduino editor is somewhat painful to work with
- Investigate more powersaving functionality. 
    - Can probably squeeze out so more power...
- Better error handling? 
- 

#### **Webpage**
- Try to create graphs directly from influxdb instead of wrapping grafana embeded.
- 
