#include <SPI.h>
#include <RH_RF95.h>

#define RFM95_CS 10
#define RFM95_RST 7
#define RFM95_INT 2

// Change to 434.0 or other frequency, must match RX's freq!
// Using 868.1 Since the probe is in europe...
#define RF95_FREQ 868.1

// Singleton instance of the radio driver
RH_RF95 rf95(RFM95_CS, RFM95_INT);

int gpioLed1 = 6;
int gpioLed2 = 9;

void setup() 
{   
  pinMode(RFM95_RST, OUTPUT);
  digitalWrite(RFM95_RST, HIGH);
  pinMode(gpioLed1, OUTPUT);
  pinMode(gpioLed2, OUTPUT);
  
  while (!Serial);
  Serial.begin(115200);
  delay(100);

  Serial.println("Starting LoRa reciever...");

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
  // Defaults after init are 434.0MHz, 13dBm, Bw = 125 kHz, Cr = 4/5, Sf = 128chips/symbol, CRC on

  // The default transmitter power is 13dBm, using PA_BOOST.
  // If you are using RFM95/96/97/98 modules which uses the PA_BOOST transmitter pin, then 
  // you can set transmitter powers from 5 to 23 dBm:

  // Set a quite conservative modem configuration to achieve longer range but with lower bitrates. 
  // Also forcing recievemode only since I dont care about sending anything back as this project is purely based on recieving data from lora
  // endpoints

  rf95.setTxPower(20, false);
  rf95.setModeRx();
  rf95.setModemConfig(rf95.Bw125Cr45Sf2048);
}

void loop()
{
  if (rf95.available())
  {
    // Should be a message for us now   
    uint8_t buf[RH_RF95_MAX_MESSAGE_LEN];
    memset(buf, 0, RH_RF95_MAX_MESSAGE_LEN);
    uint8_t len = sizeof(buf);
    digitalWrite(gpioLed1, LOW);
    digitalWrite(gpioLed2, HIGH);
    
    if (rf95.recv(buf, &len))
    {
      
      //RH_RF95::printBuffer("Received: ", buf, len);
      //Serial.print("Got: ");
      Serial.println((char*)buf);
      Serial.print("RSSI: ");
      Serial.println(rf95.lastRssi(), DEC);
      
      // Blinking some lights to indicate a package was recieved
      digitalWrite(gpioLed1, HIGH);
      digitalWrite(gpioLed2, LOW);
    }
    else
    {
      Serial.println("Receive failed");
    }
  }
}
