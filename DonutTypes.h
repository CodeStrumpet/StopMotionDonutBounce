

typedef struct {
   float motion;
   long pingOne;
   long pingTwo;
   int irRange;
} SensorsInput;


typedef enum DonutMode {
	DonutModeNone,
	DonutModeAmbient,
	DonutModeAmbientToIntrigued,
	DonutModeIntrigued,
	DonutModeAngry,
	DonutModeFurious,
	DonutModeTesting
} DonutMode;

typedef enum MotionState {
	MotionStateUndefined,
	MotionStateNoMotionNoObjects,
	MotionStateMotionWithoutObjects,
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