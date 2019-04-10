/*
  Pameseb Arduino main programm
  
  Programme permettant de récupérer les diverses données brutes et de les transférer
  au réseau LoRaWan

  Created 7 February 2019
  By Jean-Cyril Bohy
  Modified 10 april 2019
  By Jean-Cyril Bohy

  Done:
  * communication with LoRaWAN
  * UID gestion 
  * Packeting with analogSensorData
  * Packeting with metaData
  * Packeting with wind direction and speed
  
  TODO:
  * Sleep and wake up gestion
  * Gestion of pluviometry
  * data storage gestion (SD card)
  * energy optimisation
  * Downlink LoRaWAN for RTC update
  * Communication optimisation with Proximus EnCo

  https://github.com/cybo12/Pameseb-Node

*/

#include <EEPROM.h>
#include <math.h>
#include <rn2xx3.h>
#include <SoftwareSerial.h>
#include "DS3231.h"

// Using DS3231 only for test
// Init the DS3231 using the hardware interface
DS3231  rtc(SDA, SCL);

SoftwareSerial mySerial(10, 11); // RX, TX

rn2xx3 myLora(mySerial);

//enable to modification
#define DEBUG True
uint8_t VERSION = 1;
uint8_t UID = 1;
uint8_t NB_SENS_A = 4;
//
uint8_t UID_ADD = 0;
uint8_t NB_BYTES_SENS_A = uint8_t(ceil(NB_SENS_A*10/8));
//uint8_t payload_size = 9+NB_BYTES_SENS_A;
uint8_t payload_size = 7;

void setup() { 
  analogReference(INTERNAL);
  rtc.begin(); 
  //rtc.setDOW(MONDAY);     // Set Day-of-Week to SUNDAY
  //rtc.setTime(12, 10, 30);     // Set the time (24hr format)
  //rtc.setDate(8, 4, 2019); //DD MM AAAA
  #ifdef DEBUG
  Serial.begin(57600);
  while(!Serial);
  if(UID != EEPROM.read(UID_ADD)){
      EEPROM.write(UID_ADD, UID );
      Serial.println("UID Never initalised");
    }
  Serial.print("UID: ");
  Serial.println(UID);
  Serial.print("Version: ");
  Serial.println(VERSION);
  #endif
  mySerial.begin(9600); //serial port to radio
  initialize_radio();
  delay(2000);
}

void loop() {
  delay(20000);
  byte payload[payload_size];
  packeting(payload);
  myLora.txBytes(payload,payload_size);
  #ifdef DEBUG
  for(int i = 0; i < payload_size; i++)
  {
    Serial.print("byte ");
    Serial.print(i);
    Serial.print(": ");
    Serial.println(payload[i]);
  }
  #endif
}

void packeting(byte payload[]){
  uint16_t battery = 13.12 *100;//TODO
  Serial.println(battery,BIN);
  byte byte_1 = (byte) battery & 0xE0;;
  Serial.println(byte_1,BIN );
  byte byte_2 = (byte) (battery >> 8) & 0xFF;
  Serial.println(byte_2,BIN );
  Serial.println(byte_1);
  byte_2 = (byte_2 << 5) | (byte) VERSION;
  Serial.println(byte_2,BIN );
  //initalise the payload
  payload[0] = (byte) UID;
  payload[1] = byte_1;
  payload[2] = byte_2;
  UnixTime(payload);
  //analog_sensors(payload);
}

void analog_sensors(byte payload[]){
  boolean values[NB_BYTES_SENS_A*8] = {0};
  byte addToPayload[NB_BYTES_SENS_A];
  uint16_t valueRead;
  uint8_t z = 0;
  //make a array of bits from analog read (values going from 2 bytes to 10 bits)
  for(uint8_t i = 0; i < NB_SENS_A; i++)
    {
      valueRead = analogRead(i);
      Serial.print("A");
      Serial.print(i);
      Serial.print(": ");
      Serial.print(valueRead);
      Serial.print(" = ");
      Serial.println(valueRead,BIN);
      for(uint8_t x = 0; x<10;x++){
        if(true){
        values[z] = bitRead(valueRead,9-x);
        }else{
          values[z] = 0;
        }
        z++;
      }
    }
  z=0;
  Serial.println("==========");
  //check array of bits
  //for(uint8_t p = 0; p<60;p++){Serial.print(values[p]);}
  //Serial.println();
  //convert array of bits to array of bytes gain 25% of size. Gain= 2bit/byte
  for(uint8_t y=0; y<NB_BYTES_SENS_A;y++)
  {
    for(uint8_t b=0; b<8; b++){
        bitWrite(addToPayload[y],b,values[z]);
    }
  z+=1;
  }
   //concate payload and array of bytes 
memcpy(payload+9,addToPayload,sizeof(addToPayload));
}


void UnixTime(byte payload[]){
  #ifdef DEBUG
  Serial.println(rtc.getUnixTime(rtc.getTime()));
  Serial.print(rtc.getDOWStr());
  Serial.print(" ");
  Serial.print(rtc.getDateStr());
  Serial.print(" -- ");
  Serial.println(rtc.getTimeStr());
  #endif
  long unixTime = rtc.getUnixTime(rtc.getTime()) - 1546300800;
  byte date[4];
  date[0] = (unixTime >> 24) & 0xFF;
  date[1] = (unixTime >> 16) & 0xFF;
  date[2] = (unixTime >> 8) & 0xFF;
  date[3] = unixTime & 0xFF;
  memcpy(payload+3,date,sizeof(date));
}

void initialize_radio()
{
  //reset rn2483
  pinMode(12, OUTPUT);
  digitalWrite(12, LOW);
  delay(500);
  digitalWrite(12, HIGH);

  delay(100); //wait for the RN2xx3's startup message
  mySerial.flush();

  //Autobaud the rn2483 module to 9600. The default would otherwise be 57600.
  myLora.autobaud();

  //check communication with radio
  String hweui = myLora.hweui();
  while(hweui.length() != 16)
  {
    Serial.println("Communication with RN2xx3 unsuccessful. Power cycle the board.");
    Serial.println(hweui);
    delay(10000);
    hweui = myLora.hweui();
  }

  Serial.println("When using OTAA, register this DevEUI: ");
  Serial.println(myLora.hweui());
  Serial.println("RN2xx3 firmware version:");
  Serial.println(myLora.sysver());

  Serial.println("Trying to join TTN");
  bool join_result = false;

  const char *appEui = "70B3D57ED0017DB0";
  const char *appKey = "E091E2E4AAB4162594AEC999236863FE";

  join_result = myLora.initOTAA(appEui, appKey);


  while(!join_result)
  {
    Serial.println("Unable to join. Are your keys correct, and do you have TTN coverage?");
    delay(5000); //delay a minute before retry
    join_result = myLora.init();
  }
  myLora.setDR(5);
  Serial.println("Successfully joined TTN");

}
