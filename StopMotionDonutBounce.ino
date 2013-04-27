
#include "DonutTypes.h"
#include "Helpers.h"

#define NUM_SOLENOIDS 4
#define NON_MOTION_DURATION 3000
#define PING_TRACKING_OBJECT_THRESHOLD 70
#define IR_RANGE_TRACKING_OBJECT_THRESHOLD 40
#define PIR_MOTION_THRESHOLD 0.8

// PIR Sensor
int calibrationTime = 30; // the time we give the sensor to calibrate (10-60 secs according to the datasheet)
int pirPin = 10;
long unsigned int lastNonMotionTime; // use this to check if there has been recent motion

// Solenoids
int solenoidPins[] = {2, 7, 8, 9};
boolean solenoidState[] = {false, false, false, false};

// Ping sensors
int pingPins[] = {11, 12};
long pingDistances[] = {0, 0};

// IR range sensor
int irRangePin = A0;

// Structs to hold current and reference sensor values
SensorsInput currInput, referenceInput;

// low pass alpha
const float lowPassAlpha = 0.4;

// Current Mode
DonutMode currMode = DonutModeTesting;
long unsigned int enteredModeTime;

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
	processSensorsInput();

	printModeAndMotionInfo();
	printSensorValues();

	delay(150);

	adjustDonutModeForLastInput();

	for (int i = 0; i < NUM_SOLENOIDS; i++) {
		digitalWrite(solenoidPins[i], solenoidState[i]);
	}

}

void captureSensorsInput() {

	// read IR Motion sensor
	if (digitalRead(pirPin) == HIGH) {
		currInput.motion = 1.0;
	} else {
		currInput.motion = 0.0;
	}

	// read Ping sensors
	currInput.pingOne = inchesForPingPin(pingPins[0]);
	currInput.pingTwo = inchesForPingPin(pingPins[1]);

	// read IR Range sensor
	currInput.irRange = analogRead(irRangePin);
}

void processSensorsInput() {
	referenceInput = sensorsInputLowPassResult(currInput, referenceInput, lowPassAlpha);

	if (referenceInput.motion < PIR_MOTION_THRESHOLD) {
		lastNonMotionTime = millis();
	}
}

void printSensorValues() {
	/*
	Serial.print("   Curr Input: {");
	Serial.print("motion : ");
	Serial.print(currInput.motion);
	Serial.print(", pingOne : ");
	Serial.print(currInput.pingOne);
	Serial.print(", pingTwo : ");
	Serial.print(currInput.pingTwo);
	Serial.print(", irRange : ");
	Serial.print(currInput.irRange);
	Serial.print("}");
	*/
	Serial.print("   Ref Input: {");
	Serial.print("motion : ");
	Serial.print(referenceInput.motion);
	Serial.print(", pingOne : ");
	Serial.print(referenceInput.pingOne);
	Serial.print(", pingTwo : ");
	Serial.print(referenceInput.pingTwo);
	Serial.print(", irRange : ");
	Serial.print(referenceInput.irRange);
	Serial.println("}");
}

void printModeAndMotionInfo() {
	String motionStateString;

	switch(currentMotionState()) {		
		case MotionStateNoMotionNoObjects:
			motionStateString = "MotionStateNoMotionNoObjects";
			break;
		case MotionStateMotionWithoutObjects:
			motionStateString = "MotionStateMotionWithoutObjects";
			break;
		case MotionStateMotionWithObjects:
			motionStateString = "MotionStateMotionWithObjects";
			break;
		case MotionStateNoMotionWithObjects:
			motionStateString = "MotionStateNoMotionWithObjects";
			break;
		case MotionStateUndefined:
		default:
			motionStateString = "MotionStateUndefined";
			break;
	}

	Serial.print("MotionState:  ");
	Serial.print(motionStateString);

/*
	if (doesMotionExistInDonutProximity()) {
		Serial.print("MOTION!!");		
	} else {
		Serial.print("NO MOTION");
	}
*/
}

void adjustDonutModeForLastInput() {
		

	/*
	
	if (values > movementThreshold)
		lastMovementTime = millis()
		if ()





	*/
	// 
}

MotionState currentMotionState() {
	bool motionPresent = false;
	if (referenceInput.motion > PIR_MOTION_THRESHOLD) {
		motionPresent = true;
	}

	bool trackingObjects = false;
	if (referenceInput.pingOne < PING_TRACKING_OBJECT_THRESHOLD ||
		referenceInput.pingTwo < PING_TRACKING_OBJECT_THRESHOLD||
		referenceInput.irRange > IR_RANGE_TRACKING_OBJECT_THRESHOLD) {
		trackingObjects = true;
	}

	MotionState motionState = MotionStateUndefined;
	if (!motionPresent && !trackingObjects) {
		motionState = MotionStateNoMotionNoObjects;
	} else if (motionPresent && !trackingObjects) {
		motionState = MotionStateMotionWithoutObjects;
	} else if (motionPresent && trackingObjects) {
		motionState = MotionStateMotionWithObjects;
	} else if (!motionPresent && trackingObjects) {
		motionState = MotionStateNoMotionWithObjects;
	}

	return motionState;
}


bool doesMotionExistInDonutProximity() {
	bool motionExists = false;

	// we declare there is motion if ping has been active within NON_MOTION_DURATION and we are tracking an object rather than air 
	if (millis() - lastNonMotionTime < NON_MOTION_DURATION) {
		if (referenceInput.pingOne < PING_TRACKING_OBJECT_THRESHOLD ||
			referenceInput.pingTwo < PING_TRACKING_OBJECT_THRESHOLD||
			referenceInput.irRange > IR_RANGE_TRACKING_OBJECT_THRESHOLD) {
			motionExists = true;
		}
	}
	return motionExists;
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