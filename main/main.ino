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

int VERSION = 1;
int UID_ADD = 0;
int UUID = 1;
int value;

void setup() {
  Serial.begin(9600);
  while(!Serial);
  value = EEPROM.read(UID_ADD);
  if(value == 0){
      EEPROM.write(UID_ADD, UUID );
      Serial.println("UID Never initalised");
    }
  Serial.print("UID: ");
  Serial.println(value);
  Serial.print("Version: ");
  Serial.println(VERSION);
}

void loop() {
  delay(1000);
  int sensorValue = analogRead(A0);
  float voltage = sensorValue / 1023.0;
  Serial.println(voltage);
}
