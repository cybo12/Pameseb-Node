/*
  Pameseb Arduino main programm
  
  Programme permettant de récupérer les diverses données brutes et de les transférer
  au réseau LoRaWan

  The circuit:
  * input :
  * output :

  Created 7 February 2019
  By Jean-Cyril Bohy
  Modified 7 February 2019
  By Jean-Cyril Bohy

  https://github.com/cybo12/Pameseb-Node

*/

#include <EEPROM.h>
#include <math.h>
#include "DS3231.h"

// Init the DS3231 using the hardware interface
DS3231  rtc(SDA, SCL);

//enable to modification
#define DEBUG True
uint8_t VERSION = 1;
uint8_t UID = 1;
uint8_t NB_SENS_A = 4;
//
uint8_t UID_ADD = 0;
uint8_t NB_BYTES_SENS_A = uint8_t(ceil(NB_SENS_A*10/8));
//uint8_t payload_size = 9+NB_BYTES_SENS_A;
uint8_t payload_size = 9;

void setup() { 
  analogReference(INTERNAL);
  #ifdef DEBUG
  Serial.begin(9600);
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
  rtc.begin(); 
  rtc.setDOW(MONDAY);     // Set Day-of-Week to SUNDAY
  rtc.setTime(16, 41, 40);     // Set the time to 12:00:00 (24hr format)
  rtc.setDate(11, 2, 2019); //DD MM AAAA
}

void loop() {
  delay(10000);
  byte payload[payload_size];
  packeting(payload);
  #ifdef DEBUG
  Serial.println(rtc.getUnixTime(rtc.getTime()));
  Serial.println(rtc.getUnixTime(rtc.getTime()),BIN);
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
  byte byte_2 = (byte) battery;
  byte byte_3 = (byte) battery << 8;
  byte_3 = byte_3 | (byte) VERSION;
  //initalise the payload
  payload[0] = (byte) VERSION;
  payload[1] = byte_2;
  payload[2] = byte_3;
  uint16_t temp = (uint16_t)(rtc.getTemp()+40)*4;
  Serial.println(temp);
  Serial.println(temp,BIN);
  payload[3] = highByte(temp);
  payload[4] = lowByte(temp);
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
  int unixTime = rtc.getUnixTime(rtc.getTime());
  byte date[4];
  date[1] = (unixTime >> 24) & 0xFF;
  date[2] = (unixTime >> 16) & 0xFF;
  date[3] = (unixTime >> 8) & 0xFF;
  date[4] = unixTime & 0xFF;
  memcpy(payload+5,date,sizeof(date));
}