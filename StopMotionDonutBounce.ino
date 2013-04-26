
#include "DonutTypes.h"

#define NUM_SOLENOIDS 4

// PIR Sensor
int calibrationTime = 30; // the time we give the sensor to calibrate (10-60 secs according to the datasheet)
int pirPin = 10;

// Solenoids
int solenoidPins[] = {2,7,8,9};
boolean solenoidState[] = {false, false, false, false};

// Structs to hold current and reference sensor values
SensorsInput currInput, referenceInput;

// low pass alpha
const float lowPassAlpha = 0.5;

// Current Mode
DonutMode currMode = DonutModeTesting;

void setup() {

	// initialize sensor input structs
	referenceInput.motion = 0.0;


	// start serial
	Serial.begin(9600);         

	// setup Input pins
	pinMode(pirPin, INPUT);

	// setup Output pins
	for (int i = 0; i < NUM_SOLENOIDS; i++) {
		pinMode(solenoidPins[i], OUTPUT);
	}

	// calibrate Sensors
	calibratePIR();
}

void loop() {

	// capture and process sensors input
	captureSensorsInput();
	referenceInput = sensorsInputLowPassResult(currInput, referenceInput, lowPassAlpha);
	printSensorValues();

	delay(150);

	adjustDonutModeForLastInput();

	for (int i = 0; i < NUM_SOLENOIDS; i++) {
		digitalWrite(solenoidPins[i], solenoidState[i]);
	}

}

void captureSensorsInput() {
	if (digitalRead(pirPin) == HIGH) {
		currInput.motion = 1.0;
	} else {
		currInput.motion = 0.0;
	}
}

void printSensorValues() {
	Serial.print("Curr Input: {");
	Serial.print("motion : ");
	Serial.print(currInput.motion);
	Serial.print("}  Ref Input: {");
	Serial.print("motion : ");
	Serial.print(referenceInput.motion);
	Serial.println("}");
}

void adjustDonutModeForLastInput() {
	
	// 
}

int targetSolenoid = 0;

void bounceWithCurrentState() {
	if (currMode == DonutModeTesting) {
		if (targetSolenoid == NUM_SOLENOIDS) {
			targetSolenoid = 0;
		}

		for (int i = 0; i < NUM_SOLENOIDS; i++) {
			if (i == targetSolenoid) {
				solenoidState[i] = true;
			} else {
				solenoidState[i] = false;
			}
		}
		targetSolenoid++;
		delay(150);
	}
}

void calibratePIR() {
	//give the sensor some time to calibrate
	Serial.print("calibrating sensor ");
    for(int i = 0; i < calibrationTime; i++) {
      	Serial.print(".");
     	delay(1000);
    }
    Serial.println(" done");
    Serial.println("SENSOR ACTIVE");
    delay(50);
}