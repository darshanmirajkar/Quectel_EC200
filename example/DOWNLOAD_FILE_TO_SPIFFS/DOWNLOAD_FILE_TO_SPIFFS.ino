/* Example to Download file  */


#include <Arduino.h>
#include "Quectel_EC200.h"


char* downloadURL = "https://2005.filemail.com/api/file/get?filekey=8sj-7kgCWq5aq_GY1s_vM7DScpoMSZKTrHcXvcmp2j5RD6o7RId3JilWCDFfJQ";
char* filename = "/test.bin";


void setup() {
	delay(5000); // To avoid ESP32 boot Fail
  	Serial.begin(115200);
	EC200module.setup();
  	delay(2000);
}



void loop() {
  	/* Download file and store to SPIFFS as "/test/bin" */
  	EC200module.downloadFile(downloadURL, filename);
  	while(1);
  }
