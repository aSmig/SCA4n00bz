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
char readOneChar_;

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
  readOneChar_ = -1;

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
    Serial.println("To write an EEPROM: [Device Address]W[offset]D[Data]D[Data]...");
    Serial.println("    Example: 80W00D255 or 80W01D37D9D129D56D1D0D0 or 80W01D169D8D9D0D132D35D0 or 80W01D180D4D19D134");
    Serial.println("\n Pardon my bad coding, you must enter decimal numbers, and there is minimal error checking");
  }
  Serial.println("To rescan, press the reset button\n");
  Serial.print(">");
}

char readOneChar()
{
  char read_buffer[1];
  if (readOneChar_ != -1) {
    Serial.println("Flushing readOneChar_");
    char temp = readOneChar_;
    readOneChar_ = -1;
    return temp;
  } else {
    Serial.readBytes(read_buffer, 1);
    return read_buffer[0];
  }
}

byte readNumber()
{
  char c;
  char string[20];
  string[0] = 0;
  boolean done = true;
  if (Serial.available()) {
    done = false;
  }
  while (done == false) {
    c = readOneChar();
    Serial.print("                  ");
    //Serial.print("I got a char: ");
    //Serial.println(c, HEX);
    if (c >= '0' && c <= '9' ||
        c >= 'a' && c <= 'f' ||
        c >= 'A' && c <= 'F' ||
        c == 'x' ) {
          //Serial.println("Appending char");
          strncat(string, &c, 1);
         } else {
          readOneChar_ = c;  // Leftovers live here
          done = true;
        }
  }
  Serial.println("");
  Serial.print("I got a string: ");
  Serial.println(string);
  Serial.println(strtol(string, NULL, 16), HEX);
  return (byte) strtol(string, NULL, 16);
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
    rORw = readOneChar();
    if (rORw=='R')
    {
      offset = readNumber();
      Serial.print("Reading from 0x");
      Serial.println(offset,HEX);
      c = readOneChar();
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
        d = readOneChar();
        if (d==' ')
        {
          writeData = readNumber();
          Serial.print(offset,HEX);
          Serial.print(": ");
          Serial.println(writeData,HEX);
          eeprom_i2c_write(address,offset++,writeData);
        }
        else Serial.println("Error Parsing Command\n");
      }
    }
    else Serial.println("Error Parsing Command\n");
    Serial.print(">");
  }    
}

