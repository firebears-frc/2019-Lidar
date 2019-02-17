#include "Adafruit_VL53L0X.h"

/*Cargo_Hatch Lidars alert the RoboRio of the following through Discrete output pins 8, 9, 10, 11
pin 8  Cargo to the left
pin 9  Cargo to the right
pin 10 Cargo in place
pin 11 Hatch in place

pin 6 connects to the Cargo Lidar SHDN and pin 7 connects to the Hatch Lidar SHDN
*/

// address we will assign if dual sensor is present
#define HATCH_ADDRESS 0x30
#define CARGO_ADDRESS 0x31

// set the pins to shutdown
#define SHT_HATCH 6
#define SHT_CARGO 7

//set Robo Rio pins
#define CARGO_ACQ 2
#define CARGO_LEFT 3
#define CARGO_RIGHT 4
#define HATCH_ACQ 5

#define BOX_WIDTH 647//25.4 inches
#define CARGO_WIDTH 350 //13.77 inches
#define HATCH_ACQ_DIST 40 //About 1.5 inches
#define SENSOR_OFFSET 28  //about 1.125 inches

int m_cargoLeft;
int m_cargoRight;


// objects for the vl53l0x
Adafruit_VL53L0X hatchLidar = Adafruit_VL53L0X();
Adafruit_VL53L0X cargoLidar = Adafruit_VL53L0X();

// this holds the measurement
VL53L0X_RangingMeasurementData_t measure_Hatch;
VL53L0X_RangingMeasurementData_t measure_Cargo;

/*
    Reset all sensors by setting all of their XSHUT pins low for delay(10), then set all XSHUT high to bring out of reset
    Keep sensor #1 awake by keeping XSHUT pin high
    Put all other sensors into shutdown by pulling XSHUT pins low
    Initialize sensor #1 with lox.begin(new_i2c_address) Pick any number but 0x29 and it must be under 0x7F. Going with 0x30 to 0x3F is probably OK.
    Keep sensor #1 awake, and now bring sensor #2 out of reset by setting its XSHUT pin high.
    Initialize sensor #2 with lox.begin(new_i2c_address) Pick any number but 0x29 and whatever you set the first sensor to
 */
void setID() {
  // all reset
  digitalWrite(SHT_HATCH, LOW);    
  digitalWrite(SHT_CARGO, LOW);
  delay(10);
  // all unreset
  digitalWrite(SHT_HATCH, HIGH);
  digitalWrite(SHT_CARGO, HIGH);
  delay(10);

  // activating LOX1 and reseting cargoLidar
  digitalWrite(SHT_HATCH, HIGH);
  digitalWrite(SHT_CARGO, LOW);

  // initing LOX1
  if(!hatchLidar.begin(HATCH_ADDRESS)) {
    Serial.println(F("Failed to boot first VL53L0X"));
    while(1);
  }
  delay(10);

  // activating cargoLidar
  digitalWrite(SHT_CARGO, HIGH);
  delay(10);

  //initing cargoLidar
  if(!cargoLidar.begin(CARGO_ADDRESS)) {
    Serial.println(F("Failed to boot second VL53L0X"));
    while(1);
  }
}

void read_dual_sensors() {
  
  hatchLidar.rangingTest(&measure_Hatch, false); // pass in 'true' to get debug data printout!
  cargoLidar.rangingTest(&measure_Cargo, false); // pass in 'true' to get debug data printout!

  // print sensor one reading
 // Serial.print("Hatch: ");
 // if(measure_Hatch.RangeStatus != 4) {     // if not out of range
    Serial.print("Hatch : ");
    Serial.print(measure_Hatch.RangeMilliMeter);
    
    if(measure_Hatch.RangeMilliMeter < HATCH_ACQ_DIST){digitalWrite(HATCH_ACQ, HIGH);}
    else {digitalWrite(HATCH_ACQ, LOW);}
  //} else {
 //   Serial.print("Out of range");
 //}
  
  //Serial.print(" ");

  // print sensor two reading
  //Serial.print("   Cargo: ");
  //if(measure_Cargo.RangeStatus != 4) {
   
   int icapt = BOX_WIDTH - 100;
   /*uint16_t cargoDistU = measure_Cargo.RangeMilliMeter;
   int cargoDist = (int)cargoDistU;
   Serial.print(cargoDist);*/

  Serial.print("   Cargo : ");
  Serial.println(measure_Cargo.RangeMilliMeter);

   

    if(measure_Cargo.RangeMilliMeter < m_cargoRight){digitalWrite(CARGO_RIGHT, HIGH);}
    else {digitalWrite(CARGO_RIGHT, LOW);}

    if(measure_Cargo.RangeMilliMeter < icapt && measure_Cargo.RangeMilliMeter > m_cargoLeft){digitalWrite(CARGO_LEFT, HIGH);}
    else {digitalWrite(CARGO_LEFT, LOW);}

    if(measure_Cargo.RangeMilliMeter < icapt){digitalWrite(CARGO_ACQ, HIGH);}
    else {digitalWrite(CARGO_ACQ, LOW);}



  //} else {
  //  Serial.print("Out of range");
  //}
  
  Serial.println();
}

void setup() {
  Serial.begin(115200);

  int eachSideCtr =( BOX_WIDTH - CARGO_WIDTH)/2;
  m_cargoRight = eachSideCtr/2 - SENSOR_OFFSET ;

  m_cargoLeft =  BOX_WIDTH - CARGO_WIDTH - m_cargoRight - SENSOR_OFFSET;

  Serial.print(" Cargo right = ");
  Serial.print(m_cargoRight);
  Serial.print(" Cargo left = ");
  Serial.println(m_cargoLeft);


  // wait until serial port opens for native USB devices
  while (! Serial) { delay(1); }

  pinMode(SHT_HATCH, OUTPUT);
  pinMode(SHT_CARGO, OUTPUT);

  Serial.println("Shutdown pins inited...");

  digitalWrite(SHT_HATCH, LOW);
  digitalWrite(SHT_CARGO, LOW);

  Serial.println("Both in reset mode...(pins are low)");
  
  
  Serial.println("Starting...");
  setID();
 
}

void loop() {
   
  read_dual_sensors();
  delay(100);
}
