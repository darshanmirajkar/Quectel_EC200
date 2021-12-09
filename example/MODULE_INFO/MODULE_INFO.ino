/* Example to Module Information  */


#include <Arduino.h>
#include "Quectel_EC200.h"


void setup() {
	delay(5000); // To avoid ESP32 boot Fail
  	Serial.begin(115200);
	EC200module.basicSetup();

}



void loop() {
	EC200module.getModuleInfo();
  	while(1);
}
