
#include "DonutTypes.h"
#include "Helpers.h"


#define USE_CONNECTED_SENSORS true

#define NUM_SOLENOIDS 4
#define NON_MOTION_DURATION 3000
#define PING_MIN_TRACKING_OBJECT_THRESHOLD 70
#define PING_MAX_TRACKING_OBJECT_THRESHOLD 90

#define IR_RANGE_TRACKING_OBJECT_THRESHOLD 40
#define PIR_MOTION_THRESHOLD 0.4

#define ATTRACT_MODE_BOUNCE_ENABLED false
#define ATTRACT_MODE_MIN_BOUNCE_PERIOD 500
#define ATTRACT_MODE_MAX_BOUNCE_PERIOD 1000

#define ANGRY_MODE_MIN_BOUNCE_PERIOD 0
#define ANGRY_MODE_MAX_BOUNCE_PERIOD 150

#define ANGRY_MODE_MIN_COLOR_PERIOD 10
#define ANGRY_MODE_MIN_COLOR_PERIOD 20

#define DEFAULT_BOUNCE_DURATION 125

#define SERIAL_READ_INTERVAL 10

#define RED 0
#define GREEN 1
#define BLUE 2


// RGB LEDS
const int redPin = 3;    //6;
const int greenPin = 6;  //5;
const int bluePin = 5;   // 3;
long unsigned int nextAngryColorChange;

int stepInterval = 2;
unsigned long long lastStepUpdateTime = 0;
int numSteps = 256;
int fadeIndex = 0;
int fadeStartColors[] = {0, 0, 0}; 
int fadeEndColors[] = {0, 0, 0}; 			

int red, green, blue; // store current value for each color

// PIR Sensor
const int calibrationTime = 10; // the time we give the sensor to calibrate (10-60 secs according to the datasheet)
const int pirPin = 10;
long unsigned int lastNonMotionTime; // use this to check if there has been recent motion

// Solenoids
int solenoidPins[] = {2, 7, 8, 9};
boolean solenoidState[] = {false, false, false, false};
long solenoidDisabledTimes[] = {0, 0, 0, 0};
long unsigned int nextAttractDonutBounce;
long unsigned int nextAngryDonutBounce;

// Ping sensors
int pingPins[] = {11, 12};
long pingDistances[] = {0, 0};

// IR range sensor
const int irRangePin = A0;

// Structs to hold current and reference sensor values
SensorsInput currInput, referenceInput;

// low pass alpha
const float lowPassAlpha = 0.4;

// Current Mode
DonutMode currMode = DonutModeTesting;
long unsigned int enteredModeTime;
long unsigned int originalEnteredModeTime;

// Cached motionstate value
MotionState lastMotionState = MotionStateNoMotionNoObjects;

// last serial read time
long unsigned int lastSerialRead = 0;


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

	// setup LED Pins
	pinMode(redPin, OUTPUT);
	pinMode(greenPin, OUTPUT);
  	pinMode(bluePin, OUTPUT);

	// calibrate Sensors
	//calibratePIR(calibrationTime);
}

void loop() {

	if (USE_CONNECTED_SENSORS) {
		// capture and process sensors input
		captureSensorsInput();	
		processSensorsInput();
	}

	//printModeAndMotionInfo();
	//printSensorValues();

	adjustDonutModeForLastInput();
	updateLEDs();
	updateBounce();
	
	
	// activate solenoids if disabled time is in the future
	long time = millis();
	for (int i = 0; i < NUM_SOLENOIDS; i++) {
		if (solenoidDisabledTimes[i] > time) {
			digitalWrite(solenoidPins[i], true);
			//Serial.println("Bounce!");
		} else {
			digitalWrite(solenoidPins[i], false);
		}		
	}
	delay(100);
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

void adjustDonutModeForLastInput() {
		
	MotionState newMotionState;
	if (USE_CONNECTED_SENSORS) {
		newMotionState = currentSensorMotionState();
	} else {
		newMotionState = currentMotionState();	
	}
	 
	DonutMode desiredDonutMode = donutModeForMotionState(newMotionState);

	long unsigned int currTime = millis();

	if (currMode != desiredDonutMode) {

		int minModeDuration = donutModeDurations[currMode];

		// bit of a hack to ensure we can always switch out of IntriguedToAttractMode to a less angry mode
		if (currMode == DonutModeIntriguedToAngry && desiredDonutMode < DonutModeIntriguedToAngry) {
			minModeDuration = 0;
		}

		// check if we can switch from the current mode
		if (currTime - enteredModeTime > minModeDuration) {
			
			currMode = nextDonutMode(currMode, desiredDonutMode);
			enteredModeTime = currTime;		
			originalEnteredModeTime = currTime;	
			initializeStateUponEnteringMode(currMode);
		}
		
	} else {
		enteredModeTime = currTime;
	}	
}

void initializeStateUponEnteringMode(DonutMode newMode) {
	if (newMode == DonutModeAttract) {
		setNextAttractDonutBounce();
	} else if (newMode == DonutModeAngry) {
		setNextAngryColor();
		setNextAngryDonutBounce();
	} else if (newMode == DonutModeIntriguedToAngry) {
		fadeIndex = 0;
		stepInterval = donutModeDurations[DonutModeIntriguedToAngry] / numSteps;
		fadeStartColors[RED] = red;
		fadeStartColors[GREEN] = green;
		fadeStartColors[BLUE] = blue;
		fadeEndColors[RED] = 255;
		fadeEndColors[GREEN] = 0;
		fadeEndColors[BLUE] = 0;
	}
}

void setNextAttractDonutBounce() {
	long unsigned int time = millis();
	if (nextAttractDonutBounce < time) {
		nextAttractDonutBounce = time + abs(random(ATTRACT_MODE_MIN_BOUNCE_PERIOD, ATTRACT_MODE_MAX_BOUNCE_PERIOD));
	}
}

void setNextAngryDonutBounce() {
	long unsigned int time = millis();
	if (nextAngryDonutBounce < time) {
		nextAngryDonutBounce = time + abs(random(ANGRY_MODE_MIN_BOUNCE_PERIOD, ANGRY_MODE_MAX_BOUNCE_PERIOD));
	}
}

void setNextAngryColor() {
	long unsigned int time = millis();
	if (nextAngryColorChange < time) {

		nextAngryColorChange = time + abs(random(ATTRACT_MODE_MIN_BOUNCE_PERIOD, ATTRACT_MODE_MAX_BOUNCE_PERIOD));		

		int randColor = random(0, 6);

		if (randColor == 0) {
			red = random(0, 255);
			green = random(0, 255);
			blue = random(100, 255);
		} else if (randColor == 1) {
			red = random(0, 255);
			green = random(100, 255);
			blue = random(0, 255);
		} else if (randColor == 2) {
			red = 255;
			green = 0;
			blue = 0;
		} else if (randColor == 3) {
			red = 0;
			green = 255;
			blue = 0;
		} else if (randColor == 4) {
			red = 0;
			green = 255;
			blue = 255;
		} else if (randColor == 5) {
			red = 100;
			green = 255;
			blue = 200;

		} else {
			red = 200;
			green = 200;
			blue = 100;
		}
		
	}
	
}



void updateLEDs() {


	if (currMode == DonutModeAttract) {
		red = 200;
		green = 200;
		blue = 200;
	} else if (currMode == DonutModeAttractToIntrigued) {
		// TODO add fade
		red = 0;
		green = 255;
		blue = 255;
	} else if (currMode == DonutModeIntrigued) {
		red = 0;  
		green = 0;
		blue = 255;
	} else if (currMode == DonutModeIntriguedToAngry) {

		if (millis() - lastStepUpdateTime > stepInterval) {

			red = (fadeStartColors[RED] * (numSteps - fadeIndex) + fadeEndColors[RED] * fadeIndex)/numSteps;
	      	green = (fadeStartColors[GREEN] * (numSteps - fadeIndex) + fadeEndColors[GREEN] * fadeIndex)/numSteps;
	      	blue = (fadeStartColors[BLUE] * (numSteps - fadeIndex) + fadeEndColors[BLUE] * fadeIndex)/numSteps;

	      	lastStepUpdateTime = millis();
	      	fadeIndex++;
		}

	} else if (currMode == DonutModeAngry) {
		if (millis() > nextAngryColorChange) {
			setNextAngryColor();
		}
	} else if (currMode == DonutModeFurious) {
		red = 255;
		green = 0;
		blue = 0;
	}

	analogWrite(redPin, red);
	analogWrite(greenPin, green);
	analogWrite(bluePin, blue);
}

void updateBounce() {
	long time = millis();

	if (currMode == DonutModeAttract && ATTRACT_MODE_BOUNCE_ENABLED) {
		if (time > nextAttractDonutBounce) {
			int solenoid = random(0, 4);
			solenoidDisabledTimes[solenoid] = time + DEFAULT_BOUNCE_DURATION;
			setNextAttractDonutBounce();			
		}
	} else if (currMode == DonutModeAngry) {
		if (time > nextAngryDonutBounce) {
			int solenoid = random(0, 4);
			solenoidDisabledTimes[solenoid] = time + DEFAULT_BOUNCE_DURATION;
			setNextAngryDonutBounce();			
		}
	}
}

MotionState currentMotionState() {
	if (millis() - lastSerialRead > SERIAL_READ_INTERVAL && Serial.available() > 0) {
      // read the incoming byte:
      int motionMode = Serial.read();

   	 if (motionMode == 'a') {
	    lastMotionState = MotionStateNoMotionNoObjects;
	  } else if (motionMode == 'b') {
	    lastMotionState = MotionStateMotionWithObjects;
	  } else if (motionMode == 'c') {
	    lastMotionState = MotionStateNoMotionWithObjects;
	  } else {
	    lastMotionState = MotionStateUndefined;
	  }

	  lastSerialRead = millis();
  }

	return lastMotionState;
}


MotionState currentSensorMotionState() {
	if (referenceInput.pingOne < 60 && referenceInput.pingTwo < 60) {
		lastMotionState = MotionStateNoMotionWithObjects;
	} else if (referenceInput.pingOne < 60) {
		lastMotionState = MotionStateMotionWithObjects;
	} else {
		lastMotionState = MotionStateNoMotionNoObjects;
	}
	return lastMotionState;
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

	MotionState newMotionState = currentMotionState();
	DonutMode desiredDonutMode = donutModeForMotionState(newMotionState);

	Serial.print("  DonutMode:  ");
	Serial.print(donutModeDescription(currMode));
	Serial.print("  DesiredMode:  ");
	Serial.print(donutModeDescription(desiredDonutMode));
	Serial.print("  MotionState:  ");
	Serial.println(motionStateDescription(newMotionState));

}