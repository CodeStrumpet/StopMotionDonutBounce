/** 
 *	Parallax Ping Sensor
 */

void calibratePIR(int calibrationDuration) {
	//give the sensor some time to calibrate
	Serial.print("calibrating sensor ");
    for(int i = 0; i < calibrationDuration; i++) {
      	Serial.print(".");
     	delay(1000);
    }
    Serial.println(" done");
    Serial.println("SENSOR ACTIVE");
    delay(50);
}

 long durationForPingPin(int pingPin) {
	// The PING))) is triggered by a HIGH pulse of 2 or more microseconds.
  	// Give a short LOW pulse beforehand to ensure a clean HIGH pulse:
	pinMode(pingPin, OUTPUT);
	digitalWrite(pingPin, LOW);
	delayMicroseconds(2);
	digitalWrite(pingPin, HIGH);
	delayMicroseconds(5);
	digitalWrite(pingPin, LOW);

	// The same pin is used to read the signal from the PING))): a HIGH
	// pulse whose duration is the time (in microseconds) from the sending
  	// of the ping to the reception of its echo off of an object.
  	pinMode(pingPin, INPUT);

  	// get duration
  	return pulseIn(pingPin, HIGH);	
}

long microsecondsToInches(long microseconds)
{
  // According to Parallax's datasheet for the PING))), there are
  // 73.746 microseconds per inch (i.e. sound travels at 1130 feet per
  // second).  This gives the distance travelled by the ping, outbound
  // and return, so we divide by 2 to get the distance of the obstacle.
  // See: http://www.parallax.com/dl/docs/prod/acc/28015-PING-v1.3.pdf
  return microseconds / 74 / 2;
}

long microsecondsToCentimeters(long microseconds)
{
  // The speed of sound is 340 m/s or 29 microseconds per centimeter.
  // The ping travels out and back, so to find the distance of the
  // object we take half of the distance travelled.
  return microseconds / 29 / 2;
}

long inchesForPingPin(int pingPin) {

	long duration = durationForPingPin(pingPin);

  	return microsecondsToInches(duration);
}

long centimetersForPingPin(int pingPin) {

	long duration = durationForPingPin(pingPin);

  	return microsecondsToCentimeters(duration);
}
