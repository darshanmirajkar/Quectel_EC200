/* Example to OTA */


#include <Arduino.h>
#include "Quectel_EC200.h"

char* downloadURL = "https://2005.filemail.com/api/file/get?filekey=8sj-7kgCWq5aq_GY1s_vM7DScpoMSZKTrHcXvcmp2j5RD6o7RId3JilWCDFfJQ";
char* checksum = "MD5 checksum_of_file";


void setup() {
	delay(5000); // To avoid ESP32 boot Fail
  	Serial.begin(115200);
	EC200module.setup();

}



void loop() {
  	/* OTA Example */
  	EC200module.downloadUpdate(downloadURL, checksum);
  	while(1);
  }
