#include "Adafruit_VL53L0X.h"
#include <SPI.h>
//Slave SPI with adafruit VL53LOX Lidar providing millimeter readings 0-1000 mm
//SPI xfer divides mm by 4, converts to unsigned integer and continuously transfers 0 to 254 unsigned byte
//The master divides by 6.35 to produce inches with a resolution of 4 mm (.15 inches)

//SPI (slave selection for Genuino UNO is pin 10, Mega pin 53)
//I2C (UNO SDA A4, SCL A5, others shown on board)


Adafruit_VL53L0X lox = Adafruit_VL53L0X();
  
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
  Serial.begin(9600);
  // wait until serial port opens for native USB devices
  while (! Serial) {
    delay(1);
  }
 // Serial.println(F("Setup")); 
  if (!lox.begin()) {
   //5556 Serial.println(F("Failed to boot VL53L0X"));
    while(1);
  }
   // Initialize SPI Slave.
  SlaveInit();
   Serial.println(F("Setup complete"));
}



void loop() {
  VL53L0X_RangingMeasurementData_t measure;   
  //Serial.print("Reading a measurement... ");
  lox.rangingTest(&measure, false); // pass in 'true' to get debug data printout!
  float distanceMM = measure.RangeMilliMeter;
  if(distanceMM > 1015.0) {distanceMM = 0.0;}
  float distanceCOMP = distanceMM /4.0;//range 0-1000 to 0 - 250 to fit in unsigned byte
  int idistanceCOMP = (int)distanceCOMP; 
  unsigned int uidistance = (unsigned int)idistanceCOMP;
  byte udistanceBYTE = lowByte(uidistance);
  Serial.print("  mm:  ");
  Serial.print(distanceMM);
  Serial.print("  distanceCOMP: ");
  Serial.print(distanceCOMP);
  Serial.print("  unsgn distanceCOMP: ");
  Serial.print(uidistance);
  Serial.print("  byte: ");
  Serial.println(udistanceBYTE);
  
  if(MISO_Control()){
  byte rx = SPItransfer(udistanceBYTE);
  }
}
