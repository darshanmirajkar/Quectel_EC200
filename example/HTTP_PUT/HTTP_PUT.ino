/* Example to put http data to server and read response  */


#include <Arduino.h>
#include "Quectel_EC200.h"


void setup() {
	delay(5000); // To avoid ESP32 boot Fail
  	Serial.begin(115200);
	EC200module.setup();

  	delay(2000);
}



void loop() {
  	EC200module.PutHTTP("https://run.mocky.io/v3/04a417d2-5288-4daa-aa41-ce4c1d21fbea", "message",4);  // HTTPS Put
  	String response = EC200module.HTTPread();
  	Serial.println("Response = " + response);       //Espected Output Response = 200 {"status":"received"}
  	delay(10000);
}
