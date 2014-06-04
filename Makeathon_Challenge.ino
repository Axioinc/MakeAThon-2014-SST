#include <Andee.h>
#include <SPI.h>
#include <Servo.h>
#include "dht11.h"

/* Start LightSensor */
const int numReadings = 10;
int readings[numReadings];
int index = 0;
int total = 0;
int average = 0;

int sensorPin=A0;
int lightPin=2;

int lightThreshold = 750;
/* End LightSensor */

/* Start BLEName */
char newBluetoothName[] = "Axio Inc."; // New device name
char cmdReply[64]; // String buffer
char commandString[100]; // String to store the new device name and device command into one
/* End BLEName */

/* Start RainSensor */
const int dhtPin = A1;   // Is a DHT sensor
const int winPin = 4;    // Is a servo
const int errPin = 7;    // Shared reserved error pin

const int winAngle  = 120;  // Angle when window is opened
const int threshold = 50;   // Humidity %
const int frequency = 1;    // How many probes per second?
const int caliOff = -10;

int state = HIGH; // LOW means open, HIGH means closed.
int prev = 0;
dht11 DHT11;
Servo windowServo;
/* End RainSensor */

/* Start AndeeHelper */
AndeeHelper lightDisplay;
AndeeHelper lightSlider;
/* End AndeeHelper  */

bool onOffState=true;

void setup(){
  Andee.begin();
  Andee.clear();
  
  // We need to combine the new device name with the device command
  sprintf(commandString, "SET BT NAME %s", newBluetoothName);
  // Send command to change device name
  Andee.sendCommand(commandString, cmdReply);
  
  setInitialData();
  
  // Initialize output pins
  pinMode(lightPin, OUTPUT);
  
  // Uncomment below to enable debug
  Serial.begin(9600);
}

void loop(){
  getLightData();
  updateLighting();
  
  //Show data/buttons on screen
  lightDisplay.update();
  lightSlider.update();
  
  lightDisplay.setData(average);
  
  // Light Slider handling
  lightThreshold=lightSlider.getSliderValue(INT);
  
  delay(50);
}

void setInitialData() {
  lightSlider.setId(0);
  lightSlider.setType(SLIDER_IN);
  lightSlider.setLocation(0,0, FULL);
  lightSlider.setTitle("Light Activation Threshold");
  lightSlider.setSliderMinMax(0,1000,0);
  lightSlider.setSliderInitialValue(lightThreshold);
  lightSlider.setSliderNumIntervals(1001);
  lightSlider.setSliderReportMode(ON_VALUE_CHANGE);
  lightSlider.setSliderColor(THEME_BLUE_DARK);
  lightSlider.setColor(THEME_BLUE);
  
  lightDisplay.setId(1);
  lightDisplay.setType(DATA_OUT);
  lightDisplay.setTitle("Light Sensor Readings");
  lightDisplay.setLocation(1,0,FULL);
  lightDisplay.setData("");
}

void getLightData() {
  total= total - readings[index];
  readings[index] = analogRead(sensorPin);
  total= total + readings[index];
  index = index + 1;
  if (index >= numReadings)
    index = 0;
  average = total / numReadings;
  Serial.println(average);  
}

void updateLighting() {
  // Lighting relay is connected to pin 2/lightPin
  if (average <= lightThreshold) {
    // light up the light
    digitalWrite(lightPin, LOW);
  }
  else if (average > lightThreshold) {
    digitalWrite(lightPin, HIGH);
  }
}

void setupRainSensor() {
  pinMode(dhtPin, INPUT);
  pinMode(winPin, OUTPUT);
  pinMode(errPin, OUTPUT);
  
  windowServo.attach(winPin);
  digitalWrite(winPin, state);
}

void updateRainSensor() {
  switch(DHT11.read(dhtPin)) {
    case DHTLIB_OK: break;
    case DHTLIB_ERROR_CHECKSUM:
      error("Cannot probe DHT sensor: Checksum error!");
      goto fail;
      break;
    case DHTLIB_ERROR_TIMEOUT:
      error("Cannot probe DHT sensor: Tiemout occured!");
      goto fail;
      break;
    default:
      error("Cannot probe DHT sensor: Unknown error!");
      goto fail;
      break;
  }
  
  Serial.println(DHT11.humidity);
  
  prev  = state;
  state = (DHT11.humidity > threshold)?HIGH:LOW;
  
  if (state != prev)
    windowServo.write(state?30:(winAngle + caliOff + 30)); // To cut down on power-sucking write cycles
  
  fail: // Failed probe, skip all code and repeat
  delay((int)1000/frequency);
}

void error(char* description) {
  digitalWrite(errPin, HIGH);
  Serial.print("[ERROR][WINDOWS][RAINHUMIDITYSENSOR] ");
  Serial.println(description);
}
