
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
  Serial.begin(115200);
 
  myLidarLite.begin(0, true);/*
   ----------------------------------------------------------------------------
    configuration:  Default 0.
      0: Default mode, balanced performance.
      1: Short range, high speed. Uses 0x1d maximum acquisition count.
      2: Default range, higher speed short range. Turns on quick termination
          detection for faster measurements at short range (with decreased
          accuracy)
      3: Maximum range. Uses 0xff maximum acquisition count.
      4: High sensitivity detection. Overrides default valid measurement detection
          algorithm, and uses a threshold value for high sensitivity and noise.
      5: Low sensitivity detection. Overrides default valid measurement detection
          algorithm, and uses a threshold value for low sensitivity and noise.
    lidarliteAddress: Default 0x62. Fill in new address here if changed. See
      operating manual for instructions.
  */
  myLidarLite.configure(0); // Change this number to try out alternate configurations

  // wait until serial port opens for native USB devices
  while (! Serial) {
    delay(1);
  }
 
  SlaveInit();
  Serial.println(F("Setup complete"));
}

void loop() 
{
   // Take a measurement with receiver bias correction and print to serial terminal
  int _distance = myLidarLite.distance(false);
  int idistanceCm = _distance ;
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
