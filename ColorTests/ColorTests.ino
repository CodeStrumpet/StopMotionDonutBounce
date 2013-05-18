// color swirl! connect an RGB LED to the PWM pins as indicated
// in the #defines
// public domain, enjoy!
 
#define REDPIN 6
#define GREENPIN 5
#define BLUEPIN 3

#define FADEPIN 2
 
#define FADESPEED 5     // make this higher to slow down

int motionMode = -1;
 
void setup() {
  pinMode(REDPIN, OUTPUT);
  pinMode(GREENPIN, OUTPUT);
  pinMode(BLUEPIN, OUTPUT);
  
  Serial.begin(9600);
}
 
 
void loop() {


  int r, g, b;



  if (Serial.available() > 0) {
      // read the incoming byte:
      motionMode = Serial.read();
  }

  if (motionMode == 'a') {
    r = 255;
    g = 0;
    b = 0;
  } else if (motionMode == 'b') {
    r = 0;
    g = 255;
    b = 0;
  } else if (motionMode == 'c') {
    r = 0;
    g = 0;
    b = 255;
  } else {
    r = 0;
    g = 0;
    b = 0;
  }

  analogWrite(REDPIN, r);
  analogWrite(GREENPIN, g);
  analogWrite(BLUEPIN, b);
 

  /*
  // fade from blue to violet
  for (r = 0; r < 256; r++) { 
    analogWrite(REDPIN, r);
    delay(FADESPEED);
  } 
  // fade from violet to red
  for (b = 255; b > 0; b--) { 
    analogWrite(BLUEPIN, b);
    delay(FADESPEED);
  } 
  // fade from red to yellow
  for (g = 0; g < 256; g++) { 
    analogWrite(GREENPIN, g);
    delay(FADESPEED);
  } 
  // fade from yellow to green
  for (r = 255; r > 0; r--) { 
    analogWrite(REDPIN, r);
    delay(FADESPEED);
  } 
  // fade from green to teal
  for (b = 0; b < 256; b++) { 
    analogWrite(BLUEPIN, b);
    delay(FADESPEED);
  } 
  // fade from teal to blue
  for (g = 255; g > 0; g--) { 
    analogWrite(GREENPIN, g);
    delay(FADESPEED);
  } 


*/

/*

  
  int fadeValue = analogRead(FADEPIN);
  
  
  int fadeOut = map(fadeValue, 0, 1023, 0, 255);
  
  Serial.print("FadeOut:  ");
  Serial.println(fadeOut);
  
  analogWrite(REDPIN, fadeOut);
  analogWrite(GREENPIN, fadeOut);
  analogWrite(BLUEPIN, fadeOut);
  */
}