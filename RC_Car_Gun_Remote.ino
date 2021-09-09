/*
   See documentation at https://nRF24.github.io/RF24
   See License information at root directory of this library
   Author: Brendan Doherty (2bndy5)
*/

/**
   A simple example of sending data from 1 nRF24L01 transceiver to another.

   This example was written to be used on 2 devices acting as "nodes".
   Use the Serial Monitor to change each node's behavior.
*/
#include <Arduino.h>
#include <SPI.h>
#include <EEPROM.h>
#include "printf.h"
#include "transmitter.h"
#include "receiver.h"

const byte auxServoPins[] = {4,6,7,9};

int eeprom_set;
TRANSMITTER transmitter;
RECEIVER receiver;

#define TX 1
#define RX 0
bool role;

void setup()
{
  eeprom_set = EEPROM.read(255);
  Serial.begin(9600);
  if (eeprom_set == 1)
  {
    if (EEPROM.read(0))
    {
      role = TX;
      transmitter.begin();
    }
    else
    {
      role = RX;
      receiver.begin(sizeof(auxServoPins) / sizeof(auxServoPins[0]),auxServoPins);
    }
  }
  else
  {
    // Default to a RX
    EEPROM.write(0, 0);
    EEPROM.write(255, 1);
  }
} // setup

void eepromr(byte addr) //reads the specified EEPROM address and prints result to serial monitor
{
  Serial.println("Reading from EEPROM");
  byte value = EEPROM.read(addr);
  Serial.print(addr);
  Serial.print("\t");
  Serial.print(value);
  Serial.println();
  delay(100);
}

void eepromwr(byte addr) //write parsed byte from serial to EEPROM address and prints the wrttien value to serial monitor
{
  Serial.println("Reading Serial In");
  delay(100);
  while (Serial.available() > 0)
  {
    Serial.println("Writing to EEPROM");
    delay(100);
    byte val = Serial.parseInt();
    delay(100);
    EEPROM.write(addr, val);

    break;
  }
}

void check_serial_monitor() //allows entrence and exit of either function read or write
{
  while (Serial.available() > 0)
  {
    char inByte = Serial.read();

    while (inByte == 'r') //if character "r" is sent from pc to arduino enter read function
    {
      eepromr(Serial.parseInt());
      break;
    }
    while (inByte == 'w') //if character w is sent from pc to arduino enter write function
    {
      eepromwr(Serial.parseInt());
      break;
    }
  }
}

void loop()
{
  check_serial_monitor();
  if (role) {
    transmitter.loop();
  }
  else {
    receiver.loop();
  }
} // loop
