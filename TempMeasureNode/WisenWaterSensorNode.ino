#include <SPI.h>
#include <RH_RF95.h>
#include <OneWire.h> 
#include <DallasTemperature.h>
#include <LowPower.h>

//For WhisperNode
#define RFM95_CS 10
#define RFM95_RST 7
#define RFM95_INT 2
#define node_id "B"

#define voltReadControlPin A0
#define voltReadPin A6
#define maxVolt 7282

int gpioLed1 = 6;
int gpioLed2 = 9;
int gpioTempSwitch = 4;
#define one_wire_bus 3
OneWire oneWire(one_wire_bus);
DallasTemperature sensors(&oneWire);


char outstr[32] = "";
char temp[10] = "";
char volt[10] = "";
char location[] = "selura:1:";
char temptype[] = "temp:";

// Counter intializer
uint8_t sleepCount = 1;
uint8_t counter = 1; 

// Change to 434.0 or other frequency, must match RX's freq!
// Using 868.1 Since the probe is in europe...
#define RF95_FREQ 868.1
 
// Singleton instance of the radio driver
RH_RF95 rf95(RFM95_CS, RFM95_INT);
//RH_RF95 rf95();


float temp_read()
{
  digitalWrite(gpioTempSwitch, HIGH);
  LowPower.powerDown(SLEEP_120MS, ADC_OFF, BOD_OFF);
  sensors.requestTemperatures();
  sensors.getTempCByIndex(0);
  float tmpFloat = (sensors.getTempCByIndex(0));
  if (tmpFloat == 85)
  {
    //Serial.println("Temp is 85, probably due to short read time, waiitng longer");
    //Had some issues where the delay was not set long enough which would result in the temperature probe showing exactly 85 degrees.
    LowPower.powerDown(SLEEP_250MS, ADC_OFF, BOD_OFF);
    sensors.getTempCByIndex(0);
    tmpFloat = (sensors.getTempCByIndex(0));
    digitalWrite(gpioTempSwitch, LOW);
    return (tmpFloat);
  }
  else
  {
    digitalWrite(gpioTempSwitch, LOW);
    return (tmpFloat);
  }
}


float volt_read()
{
  // Reads the voltage from the battery terminal on the Wisen whispernode
  // Probably dont need to repeat the analogVoltage references...
  analogReference(INTERNAL);
  digitalWrite(voltReadControlPin, HIGH);
  float volt = analogRead(voltReadPin);
  digitalWrite(voltReadControlPin, LOW);
  float calcvolt = (maxVolt * (volt / 1023000));
  return (calcvolt);
}


void setup() 
{
  pinMode(RFM95_RST, OUTPUT);
  digitalWrite(RFM95_RST, HIGH);
  pinMode(gpioTempSwitch, OUTPUT);
  pinMode(gpioLed1, OUTPUT);
  pinMode(gpioLed2, OUTPUT);
  
  Serial.begin(115200);
  while (!Serial) {
    delay(1);
  }
 
  delay(100);
 
  Serial.println("Starting LoRa Temperature transmitter...");
 
  // manual reset
  digitalWrite(RFM95_RST, LOW);
  delay(10);
  digitalWrite(RFM95_RST, HIGH);
  delay(10);
 
  while (!rf95.init()) {
    Serial.println("LoRa radio init failed");
    while (1);
  }
  Serial.println("LoRa radio initalized succesfully...");
 
  // Defaults after init are 434.0MHz, modulation GFSK_Rb250Fd250, +13dbM
  
  if (!rf95.setFrequency(RF95_FREQ)) {
    Serial.println("setFrequency failed");
    while (1);
  }
  Serial.print("Set Freq to: "); Serial.println(RF95_FREQ);
  
  // Defaults after init are 434.0MHz, 13dBm, Bw = 125 kHz, Cr = 4/5, Sf = 128chips/symbol, CRC on
 
  // The default transmitter power is 13dBm, using PA_BOOST.
  // If you are using RFM95/96/97/98 modules which uses the PA_BOOST transmitter pin, then 
  // you can set transmitter powers from 5 to 23 dBm:
  //rf95.setSpreadingFactor(9);

  // This modem configuration was tested due to some issues with longer range. Seemed to work quite well but resultet in higher battery 
  // drainage since the transmitter had to transmit for a longer period. 
  /*
  RH_RF95::ModemConfig modem_config = {
  0x78, // Reg 0x1D: BW=125kHz, Coding=4/8, Header=explicit
  0xc4, // Reg 0x1E: Spread=4096chips/symbol, CRC=enable
  0x0c  // Reg 0x26: LowDataRate=On, Agc=On
  };
  rf95.setModemRegisters(&modem_config);
  */
  
  // Set a quite conservative modem configuration to achieve longer range but with lower bitrates. 

  
  rf95.setTxPower(20, false);
  rf95.setModeTx();
  rf95.setModemConfig(rf95.Bw125Cr45Sf2048);
  
  //Serial.print(rf95.Bw125Cr45Sf2048);
}
 
void loop()
{
  
  LowPower.powerDown(SLEEP_8S, ADC_OFF, BOD_OFF);
  // Sleepcounter to send at around 10 minutes. (I dont need temperature data more often)
  if(sleepCount == 75)
    {
      
      float temperature = temp_read();
      dtostrf(temperature, 4, 2, temp);
      int lengthtemp = sprintf(outstr, "%stemp:%s",location, temp);;
      Serial.println(outstr);
      delay(10);
      rf95.send((uint8_t *)outstr,lengthtemp);
      digitalWrite(gpioLed1, HIGH);
      
      float voltage = volt_read();
      dtostrf(voltage, 4, 2, volt);
      int lengthvolt = sprintf(outstr, "%svolt:%s",location, volt);;
      Serial.println(outstr);
      delay(10);
      rf95.send((uint8_t *)outstr,lengthvolt);
      digitalWrite(gpioLed2, HIGH);
      
      // Resetting sleep counter
      sleepCount = 0;
      rf95.sleep();
      
    }
    Serial.println(counter);
    digitalWrite(gpioLed1, LOW);
    digitalWrite(gpioLed2, LOW);
    counter++;
    sleepCount++;
}
