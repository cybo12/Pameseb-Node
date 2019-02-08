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

#define DEBUG True

uint8_t VERSION = 1;
uint8_t UID = 1;
uint8_t NB_SENS_A = 6;

uint8_t UID_ADD = 0;
uint8_t NB_BYTES_SENS_A = uint8_t(ceil(NB_SENS_A*10/8));
uint8_t payload_size = 3+NB_BYTES_SENS_A;

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
}

void loop() {
  delay(10000);
  byte payload[payload_size];
  packeting(payload);
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
  byte byte_2 = (byte) battery;
  byte byte_3 = (byte) battery << 8;
  byte_3 = byte_3 | (byte) VERSION;
  //initalise the payload
  payload[0] = (byte) VERSION;
  payload[1] = byte_2;
  payload[2] = byte_3;
  boolean values[NB_SENS_A*10];
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
        values[z] = bitRead(valueRead,x);
        }else{
          values[z] = 0;
        }
        z++;
      }
    }
  z=0;
  //check array of bits
  for(uint8_t p = 0; p<60;p++){Serial.print(values[p]);}
  Serial.println();
  //convert array of bits to array of bytes gain 25% of size. Gain= 2bit/byte
  for(uint8_t y=0; y<NB_BYTES_SENS_A;y++)
  {
    for(uint8_t b=0; b<8; b++){
      if(y*8+b < NB_SENS_A*10){
        addToPayload[y] |= values[b] << b;
      }else{
        addToPayload[y] |= 0 << b;
      }
    }
  }
 //concate payload and array of bytes 
memcpy(payload+3,addToPayload,sizeof(addToPayload));
}
