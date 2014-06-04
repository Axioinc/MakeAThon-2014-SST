#include <Andee.h>
#include <SPI.h>

/* Start LightSensor */
const int numReadings = 10;
int readings[numReadings];
int index = 0;
int total = 0;
int average = 0;

int sensorPin=A0;
int lightPin=2;

int threshold = 750;
/* End LightSensor */

/* Start BLEName */
char newBluetoothName[] = "Axio Inc."; // New device name
char cmdReply[64]; // String buffer
char commandString[100]; // String to store the new device name and device command into one
/* End BLEName */

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
  threshold=lightSlider.getSliderValue(INT);
  
  delay(50);
}

void setInitialData() {
  lightSlider.setId(0);
  lightSlider.setType(SLIDER_IN);
  lightSlider.setLocation(0,0, FULL);
  lightSlider.setTitle("Light Activation Threshold");
  lightSlider.setSliderMinMax(0,1000,0);
  lightSlider.setSliderInitialValue(threshold);
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
  if (average <= threshold) {
    // light up the light
    digitalWrite(lightPin, LOW);
  }
  else if (average > threshold) {
    digitalWrite(lightPin, HIGH);
  }
}
