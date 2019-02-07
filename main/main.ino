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

#define DEBUG True

int VERSION = 1;
int UID_ADD = 0;
int UID = 1;
int payload_size = 3;

void setup() {
  #ifdef DEBUG
  Serial.begin(9600);
  while(!Serial);
  int value = EEPROM.read(UID_ADD);
  if(value == 0){
      EEPROM.write(UID_ADD, UID );
      Serial.println("UID Never initalised");
    }
  Serial.print("UID: ");
  Serial.println(value);
  Serial.print("Version: ");
  Serial.println(VERSION);
  #endif
}

void loop() {
  delay(1000);
  byte payload[payload_size];
  int sensorValue = analogRead(A0);
  float voltage = sensorValue / 1023.0;
  packeting(payload);
  #ifdef DEBUG
  //Serial.println(voltage);
  for(int i = 0; i < payload_size; i++)
  {
    Serial.println(payload[i]);
  }
  #endif
}

void packeting(byte Payload[]){
  Payload[0] = (byte) EEPROM.read(UID_ADD);
  Payload[1] = (byte) VERSION;
  Payload[2] = (byte) 0;//TODO
}
