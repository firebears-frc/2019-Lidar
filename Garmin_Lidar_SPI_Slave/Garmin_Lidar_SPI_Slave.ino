
#include <SPI.h>
#include <Wire.h>
#include <LIDARLite.h>

LIDARLite myLidarLite;


  
int distance;
boolean SSlast = LOW;

void SlaveInit(void) {
  // Initialize SPI pins.
  pinMode(SCK, INPUT);
  pinMode(MOSI, INPUT);
  pinMode(MISO, INPUT);
  pinMode(SS, INPUT);
  // Enable SPI as slave.
  SPCR = (1 << SPE);
}

// SPI Transfer.
byte SPItransfer(byte value) {
  SPDR = value;
  while(!(SPSR & (1<<SPIF)));
  delay(10);
  return SPDR;
}

bool MISO_Control(){
 if (!digitalRead(SS)) {
    // Yes, first time?
    if (SSlast == HIGH) {
      // Yes, take MISO pin.
      pinMode(MISO, OUTPUT);
      // Update SSlast.
      SSlast = LOW;
    }  
    return true;
  }
  else {
    // No, first time?
    if (SSlast == LOW) {
      // Yes, release MISO pin.
      pinMode(MISO, INPUT);
      //Serial.println("Slave Disabled.");
      // Update SSlast.
      SSlast = HIGH;
    }
    return false;
  }
}

void setup() { 
  myLidarLite.begin(0, true); // Set configuration to default and I2C to 400 kHz

  myLidarLite.write(0x02, 0x0d); // Maximum acquisition count of 0x0d. (default is 0x80)
  myLidarLite.write(0x04, 0b00000100); // Use non-default reference acquisition count
  myLidarLite.write(0x12, 0x03); // Reference acquisition count of 3 (default is 5)


  Serial.begin(9600);
  // wait until serial port opens for native USB devices
  while (! Serial) {
    delay(1);
  }

  SlaveInit();
  Serial.println(F("Setup complete"));
}



void loop() {

   // Take a measurement with receiver bias correction and print to serial terminal
  int _distance = distanceFast(true);
  int idistanceCm = _distance - 24;
  //Restrict range  1 -250 cm. If > 250 send 255 so reciever knows out of range
  if(idistanceCm > 250) {idistanceCm = 255;} // Max distance 250 cm = 100 In or 8ft
  if(idistanceCm < 1) {idistanceCm = 1;} // Min distance 1 cm. 
  unsigned int uidistanceCm = (unsigned int)idistanceCm;
  byte udistanceBYTE = lowByte(uidistanceCm);
  Serial.print("  Cm:  ");
  Serial.print(idistanceCm);
  Serial.print("  byte: ");
  Serial.println(udistanceBYTE);
  
  if(MISO_Control()){
  byte rx = SPItransfer(udistanceBYTE);
  } 
}

int distanceFast(bool biasCorrection)
{
  byte isBusy = 1;
  int distance;
  int loopCount;

  // Poll busy bit in status register until device is idle
  while(isBusy)
  {
    // Read status register
    Wire.beginTransmission(LIDARLITE_ADDR_DEFAULT);
    Wire.write(0x01);
    Wire.endTransmission();
    Wire.requestFrom(LIDARLITE_ADDR_DEFAULT, 1);
    isBusy = Wire.read();
    isBusy = bitRead(isBusy,0); // Take LSB of status register, busy bit

    loopCount++; // Increment loop counter
    // Stop status register polling if stuck in loop
    if(loopCount > 9999)
    {
      break;
    }
  }

  // Send measurement command
  Wire.beginTransmission(LIDARLITE_ADDR_DEFAULT);
  Wire.write(0X00); // Prepare write to register 0x00
  if(biasCorrection == true)
  {
    Wire.write(0X04); // Perform measurement with receiver bias correction
  }
  else
  {
    Wire.write(0X03); // Perform measurement without receiver bias correction
  }
  Wire.endTransmission();

  // Immediately read previous distance measurement data. This is valid until the next measurement finishes.
  // The I2C transaction finishes before new distance measurement data is acquired.
  // Prepare 2 byte read from registers 0x0f and 0x10
  Wire.beginTransmission(LIDARLITE_ADDR_DEFAULT);
  Wire.write(0x8f);
  Wire.endTransmission();

  // Perform the read and repack the 2 bytes into 16-bit word
  Wire.requestFrom(LIDARLITE_ADDR_DEFAULT, 2);
  distance = Wire.read();
  distance <<= 8;
  distance |= Wire.read();

  // Return the measured distance
  return distance;
}
