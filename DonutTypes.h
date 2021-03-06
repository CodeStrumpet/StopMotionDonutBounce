#include "Arduino.h"

typedef struct {
   float motion;
   long pingOne;
   long pingTwo;
   int irRange;
} SensorsInput;


typedef enum DonutMode {
	DonutModeNone = 0,
	DonutModeAttract = 1,
	DonutModeAttractToIntrigued = 2,
	DonutModeIntrigued = 3,
	DonutModeIntriguedToAngry = 4,
	DonutModeAngry = 5,
	DonutModeFurious = 6,
	DonutModeTesting = 7,
	DonutModeSafe = 8
} DonutMode;

int donutModeDurations[] = {1000,   // none
							2000,	// attract							
							2000, 	// attractToIntrigued
							200,	// intrigued
							3000,	// intrigedToAngry
							2000,	// angry
							3000,	// furious
							3000,	// testing
							3000, 	// safe
							};

typedef enum MotionState {
	MotionStateUndefined,
	MotionStateNoMotionNoObjects,
	MotionStateMotionWithObjects,
	MotionStateNoMotionWithObjects,
} MotionState;

SensorsInput sensorsInputLowPassResult(SensorsInput newInput, SensorsInput refInput, float alpha) {
	refInput.motion = (newInput.motion * alpha) + (refInput.motion * (1.0 - alpha));
	refInput.pingOne = (((float)newInput.pingOne) * alpha) + (((float)refInput.pingOne) * (1.0 - alpha));
	refInput.pingTwo = (((float)newInput.pingTwo) * alpha) + (((float)refInput.pingTwo) * (1.0 - alpha));
	refInput.irRange = (((float)newInput.irRange) * alpha) + (((float)refInput.irRange) * (1.0 - alpha));
	return refInput;
}

DonutMode donutModeForMotionState(MotionState motionState) {
	DonutMode donutMode = DonutModeNone;
	if (motionState == MotionStateNoMotionNoObjects) {
		donutMode = DonutModeAttract;
	} else if (motionState == MotionStateMotionWithObjects) {
		donutMode = DonutModeIntrigued;
	} else if (motionState == MotionStateNoMotionWithObjects) {
		donutMode = DonutModeAngry;
	}
	return donutMode;
}

DonutMode nextDonutMode(DonutMode currentMode, DonutMode desiredMode) {
	DonutMode newDonutMode;
	if (currentMode == DonutModeNone) {
		newDonutMode = desiredMode;
	} else if (currentMode == DonutModeAttract && desiredMode == DonutModeIntrigued) {
		newDonutMode = DonutModeAttractToIntrigued;
	} else if (currentMode == DonutModeAttractToIntrigued && desiredMode == DonutModeIntrigued) {
		newDonutMode = DonutModeIntrigued;
	}else if (currentMode == DonutModeAttractToIntrigued && desiredMode == DonutModeAngry) {
		newDonutMode = DonutModeIntrigued;
	} else if (currentMode == DonutModeIntrigued && desiredMode == DonutModeAngry) {
		newDonutMode = DonutModeIntriguedToAngry;
	} else if (currentMode == DonutModeIntriguedToAngry && desiredMode == DonutModeAngry) {
		newDonutMode = DonutModeAngry;
	} else if (currentMode == DonutModeIntriguedToAngry && desiredMode == DonutModeIntrigued) {
		newDonutMode = DonutModeIntrigued;
	} else if (currentMode == DonutModeIntriguedToAngry && desiredMode == DonutModeAttract) {
		newDonutMode = DonutModeAttract;
	} else if (currentMode == DonutModeAttract && desiredMode == DonutModeAngry) {
		newDonutMode = DonutModeAttractToIntrigued; // don't switch straight to angry
	} else if (desiredMode == DonutModeFurious) {
		newDonutMode = DonutModeFurious;
	} else {
		newDonutMode = desiredMode;
	}
	return newDonutMode;
}

String motionStateDescription(MotionState motionState) {
	String motionStateString;

	switch(motionState) {		
		case MotionStateNoMotionNoObjects:
			motionStateString = "MotionStateNoMotionNoObjects";
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

	return motionStateString;

}

String donutModeDescription(DonutMode donutMode) {
	String donutModeString;

	switch(donutMode) {		
		case DonutModeNone:
			donutModeString = "DonutModeNone";
			break;
		case DonutModeAttract:
			donutModeString = "DonutModeAttract";
			break;
		case DonutModeAttractToIntrigued:
			donutModeString = "DonutModeAttractToIntrigued";
			break;
		case DonutModeIntrigued:
			donutModeString = "DonutModeIntrigued";
			break;
		case DonutModeIntriguedToAngry:
			donutModeString = "DonutModeIntriguedToAngry";
			break;
		case DonutModeAngry:
			donutModeString = "DonutModeAngry";
			break;
		case DonutModeFurious:
			donutModeString = "DonutModeFurious";
			break;
		case DonutModeTesting:
			donutModeString = "DonutModeTesting";
			break;
		case DonutModeSafe:
			donutModeString = "DonutModeSafe";
			break;
		default:
			donutModeString = "Unknown";
			break;
	}

	return donutModeString;
}