// --------------------------------------
// i2c_scanner
//
// Version 1
//    This program (or code that looks like it)
//    can be found in many places.
//    For example on the Arduino.cc forum.
//    The original author is not know.
// Version 2, Juni 2012, Using Arduino 1.0.1
//     Adapted to be as simple as possible by Arduino.cc user Krodal
// Version 3, Feb 26  2013
//    V3 by louarnold
// Version 4, March 3, 2013, Using Arduino 1.0.3
//    by Arduino.cc user Krodal.
//    Changes by louarnold removed.
//    Scanning addresses changed from 0...127 to 1...119,
//    according to the i2c scanner by Nick Gammon
//    http://www.gammon.com.au/forum/?id=10896
// Version 5, March 28, 2013
//    As version 4, but address scans now to 127.
//    A sensor seems to use address 120.
// Version 6, July 25, 2013
//    This version modified by joefitz@securinghardware.com to add interactive eeprom read/write from serial console
//
// This sketch tests the standard 7-bit addresses
// Devices with higher bit address might not be seen properly.
//

#include <Wire.h>
#define DELAY_MS 10

void eeprom_i2c_write(byte address, byte from_addr, byte data) {
  Wire.beginTransmission(address);
  Wire.write(from_addr);
  Wire.write(data);
  Wire.endTransmission();
}

byte eeprom_i2c_read(int address, int from_addr) {
  Wire.beginTransmission(address);
  Wire.write(from_addr);
  Wire.endTransmission();

  Wire.requestFrom(address, 1);
  if(Wire.available())
    return Wire.read();
  else
    return 0xFF;
}


void setup()
{
  Serial.begin(9600);
  Wire.begin();

  byte error, address;
  int nDevices;

  return;
  Serial.println("Scanning for I2C devices...");

  nDevices = 0;
  for(address = 1; address < 127; address++ ) 
  {
    // The i2c_scanner uses the return value of
    // the Write.endTransmisstion to see if
    // a device did acknowledge to the address.
    Wire.beginTransmission(address);
    error = Wire.endTransmission();

    if (error == 0)
    {
      Serial.print("I2C device found at address 0x");
      if (address<16) 
        Serial.print("0");
      Serial.print(address,HEX);
      Serial.println("  !");

      nDevices++;
    }
    else if (error==4) 
    {
      Serial.print("Unknow error at address 0x");
      if (address<16) 
        Serial.print("0");
      Serial.println(address,HEX);
    }    
  }
  if (nDevices == 0)
    Serial.println("No I2C devices found.\n");
  else
  {
    Serial.println("done.\n");
    Serial.println("To read an EEPROM: [Device Address]R[offset]N[# bytes]");
    Serial.println("    Example: 80R00 or 80R00N8");
    Serial.println("To write an EEPROM: [Device Address]W[offset][Space][Data][Space][Data]...");
    Serial.println("    Example: 80W00 255 or 80W01 37 9 129 56 1 0 0 or 80W01 169 8 9 0 132 35 0 or 80W01 180 4 19 134");
    Serial.println("\nNumbers starting with: 0 will be parsed as Octal");
    Serial.println("    0 will be parsed as Octal");
    Serial.println("    0x will be parsed as Hexadecimal");
    Serial.println("    any other digits will be parsed as Decimal");
    Serial.println("\nPardon my bad coding, there is minimal error checking");
  }
  Serial.println("To rescan, press the reset button\n");
  Serial.print(">");
}

void serialAvailableBlock() {
  while (Serial.available() < 1) {
    delay(DELAY_MS);
  }
}  

char blockingRead()
{
  serialAvailableBlock();
  return Serial.read();
}

char blockingPeek()
{
  serialAvailableBlock();
  return Serial.peek();
}

byte readNumber()
{
  char c;
  char string[20];
  string[0] = 0;
  boolean done = false;
  while (done == false) {
    c = blockingRead();
    strncat(string, &c, 1);
    //Serial.print("Building string: ");
    //Serial.println(string);
    c = blockingPeek();
    //Serial.print("                             ");
    //Serial.print("Peek at next char: ");
    //Serial.println(c);
    if (c >= '0' && c <= '9' ||
      c >= 'a' && c <= 'f' ||
      c >= 'A' && c <= 'F' ||
      c == 'x' )
    {
      //Serial.println("More to come");
      // 0x45R0x67
    } 
    else {
      //Serial.print("Non-number character is next: 0x");
      //Serial.println(c, HEX);
      done = true;
    }
  }
  //Serial.println("");
  //Serial.print("The final string is: ");
  //Serial.print(string);
  //Serial.print(" with value: ");
  //Serial.println(strtol(string, NULL, 0), HEX);
  return (byte) strtol(string, NULL, 0);
}

void loop()
{
  byte address,offset,count;
  byte rORw,d,c;
  byte writeData;
  if (Serial.available() > 0) {
    address = readNumber();
    Serial.print("Device 0x");
    Serial.print(address,HEX);
    Serial.println(":");
    rORw = blockingRead();
    if (rORw=='R')
    {
      offset = readNumber();
      Serial.print("Reading from 0x");
      Serial.println(offset,HEX);
      c = blockingRead();
      if (c=='N')
      {
        count = readNumber();
        Serial.print("bytes to read:");
        Serial.println(count);
      }
      else count=1;
      for (int i=0;i<count;i++)
      {
        Serial.print(offset+i,HEX);
        Serial.print(": ");
        Serial.println(eeprom_i2c_read(address,offset+i),HEX);
      }
    }
    else if (rORw=='W')
    {
      offset = readNumber();
      while(Serial.available()>0)
      {
        d = blockingRead();
        if (d==' ')
        {
          writeData = readNumber();
          Serial.print(offset,HEX);
          Serial.print(": ");
          Serial.println(writeData,HEX);
          eeprom_i2c_write(address,offset++,writeData);
        }
        else if (c < ' ') {
          // Ignore Newline or Carriage return
        }
        else Serial.println("Error Parsing Command\n");
      }
    }
    else Serial.println("Error Parsing Command\n");
    Serial.print(">");
  }    
}


