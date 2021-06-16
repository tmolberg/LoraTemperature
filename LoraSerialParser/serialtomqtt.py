#!/usr/bin/python3.6
import time
import serial
import sys
import paho.mqtt.client as mqtt
from datetime import datetime
from influxdb_client import InfluxDBClient, Point
from influxdb_client.client.write_api import SYNCHRONOUS


# Remember to change the varibles below to match your influx database and mqtt server...
influxaddress = 'redacted'
influxdatabase = 'redacted'
influxtoken = 'redacted'
influxorg = 'redacted'
bucket = "redacted"

# Timeout value is in MS and not as the usual seconds...
# Should probably add some errorchecking here...
client = InfluxDBClient(url=influxaddress, token=influxtoken, org=influxorg, timeout=10000)
write_api = client.write_api(write_options=SYNCHRONOUS)

# writing to a log in the working area. Just to get some info on how often we are recieving messages
# Not actually using the log area at this time... Should probably reimplement this
logplace = "mqttparser.log"
timestring = datetime.now()
now = timestring.strftime("%d/%m/%Y %H:%M:%S - ")
log=open(logplace, "a+")

# Change serialport to the appropriate port
ser = serial.Serial(
 #port='/dev/serial0',
 port='/dev/ttyUSB0',
 baudrate = 115200,
 parity=serial.PARITY_NONE,
 stopbits=serial.STOPBITS_ONE,
 bytesize=serial.EIGHTBITS,
 timeout=1
)

ser.reset_input_buffer()
counter=0
log.write(now + "Serial to MqttServer running - Check mqtt server for packets")

# mqttclient is currently not in use... reshould also be implemented...
mqttclient = mqtt.Client(client_id="lora", clean_session=True, userdata=None, transport="tcp")
try:
    mqttclient.connect("redacted", port=1883, keepalive=60, bind_address="")
except:
    print("Unable to connect to mqttserver...")
    sys.exit()

mqttclient.loop_start()
time.sleep(2)

def InfluxPipeline(measurement, value, location, nodeid):
    print (measurement, value, location, nodeid)
    p = Point(measurement).tag("location", location).tag("nodeid", nodeid).field(measurement, value)
    try:
        write_api.write(bucket=bucket, record=p)
    except:
        print("Unable to write to influxdb...")    

def MqttMsgData(sensorName, sensorNumber, sensorType, sensorValue):
    device = "devices/lora/" + sensorName+"/" + sensorNumber+"/" + sensorType
    log.write(now + device + "\n")
    mqttclient.publish(device, sensorValue)
    print (now + "DEBUG - MQTT message published")
    print (now + "DEBUG - " + device + " - Value: " + sensorValue)

while 1:
    serialdata=ser.readline()
    timestring = datetime.now()
    now = timestring.strftime("%d/%m/%Y %H:%M:%S - ")
    message = ""
    #print (serialdata)
    if serialdata == (b"\n" or serialdata == b"\r" or serialdata == b"\r\n"):
        print ("Didnt get any parsable message...")
    elif serialdata != b'':
        try:
            # Parsing the incoming message from the arduino/whispernode
            message = (serialdata.decode('UTF-8').strip("\n\r"))
            print (message)
                rssi = message.split(":")
                print ("Last message RSSI value is: " + rssi[1])
            # If we didnt get a minimum of 3 ":" we can assume the message is toast.
            # location:nodeid:nodetype:value
            # Typically the message will be built up like this;
            # randomcity:1:temp:24.14 
            elif len(message.split(":")) > 3:
                location = message.split(":")[0]
                nodeid = message.split(":")[1]
                nodetype = message.split(":")[2]
                value = float(message.split(":")[3])
                InfluxPipeline(nodetype, value,location,nodeid)
            else:
                print("Did not recognize message... See printout...")
                print (message)

        except Exception as e:
            ser.flushInput()
            print (serialdata)
            print (e)
            print ("Unable to decode data...")

        
        
        