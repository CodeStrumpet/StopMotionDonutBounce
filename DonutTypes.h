

typedef struct {
   float motion;
   int    y;
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


SensorsInput sensorsInputLowPassResult(SensorsInput newInput, SensorsInput refInput, float alpha) {
	refInput.motion = (newInput.motion * alpha) + (refInput.motion * (1.0 - alpha));
	return refInput;
}