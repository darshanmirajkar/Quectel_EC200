#include "Quectel_EC200.h"

QuectelEC200module::QuectelEC200module()
{
	_Serial = NULL;
}

void QuectelEC200module::SelectSerial(HardwareSerial *theSerial)
{
	_Serial = theSerial;
}

void QuectelEC200module::enable()
{
	pinMode(EC21_RESET, OUTPUT);
	digitalWrite(EC21_RESET, HIGH);
}

void QuectelEC200module::disable()
{
	pinMode(EC21_RESET, OUTPUT);
	digitalWrite(EC21_RESET, LOW);
}

bool QuectelEC200module::hardRestart()
{
	digitalWrite(EC21_RESET, LOW);
	delay(1000);
	digitalWrite(EC21_RESET, HIGH);
}

void QuectelEC200module::begin()
{
	_baud = DEFAULT_BAUD_RATE; // Default baud rate 115200
	_Serial->begin(_baud);
	_buffer.reserve(BUFFER_RESERVE_MEMORY);
}

void QuectelEC200module::Retry(uint16_t NumofRetrys, uint16_t RetryDelays)
{
	NumofRetry = NumofRetrys;
	RetryDelay = RetryDelays;
}

void QuectelEC200module::begin(uint32_t baud)
{
	_baud = baud;
	_Serial->begin(_baud);
	_buffer.reserve(BUFFER_RESERVE_MEMORY);
}

void QuectelEC200module::begin(uint32_t baud, uint32_t config, int8_t rxPin, int8_t txPin)
{
	_baud = baud;
	_Serial->begin(_baud, config, rxPin, txPin);
	_buffer.reserve(BUFFER_RESERVE_MEMORY);
}

bool QuectelEC200module::setup(uint32_t baudRate, int RxPin, int TxPin)
{
	SelectSerial(LTE_SERIAL_PORT); // Select the serial port
	delay(500);
	begin(baudRate, SERIAL_8N1, RxPin, TxPin);
	initilize = initilizeModule();
	if (initilize)
	{
		Serial.println("Searching For network..");
		if (checkforNetwork())
		{
			initilize = true;
			Serial.println("Network Found");
		}
		else
		{
			initilize = false;
			Serial.print(".");
		}

		if (enableECHO())
		{
#if ENABLE_DEBUG
			Serial.println("Echo Enabled");
#endif
		}
		return true;
	}
	else
	{
		Serial.println("Cellular Module Not Found");
		return false;
	}
}

bool QuectelEC200module::configureNetwork()
{
	if (disConnectNetwork())
	{
#if ENABLE_DEBUG
		Serial.println("Disconnected to internet");
#endif
	}
	if (setAPN(operaterAPN, operaterUsername, operaterPassword, operaterAuth))
	{
#if ENABLE_DEBUG
		Serial.println("Set Operater APN");
#endif
	}

	if (connectNetwork())
	{
		Serial.println("Connected to Internet");
		return true;
	}
	else
	{
		Serial.println("Failed to connect Internet");
		return false;
	}
}

void QuectelEC200module::basicSetup()
{
	SelectSerial(LTE_SERIAL_PORT); // Select the serial port
	begin(115200, SERIAL_8N1, LTE_RX_PIN, LTE_TX_PIN);
	initilizeModule();
	Serial.println("Searching For network..");
	if (initilize)
	{
		if (checkforNetwork())
		{
			Serial.println("Network Found");
		}
		else
		{
			Serial.print(".");
		}
		if (enableECHO())
		{
#if ENABLE_DEBUG
			Serial.println("Echo Enabled");
#endif
		}
	}
}

bool QuectelEC200module::SetAT()
{
	_Serial->flush();
	int count = 0;
	do
	{
		_Serial->print(F("AT\r\n"));
		_buffer = _readSerial(SET_AT_RESP_TIMEOUT);
		count++;
		delay(RetryDelay);
	} while ((count < NumofRetry && count < MAX_Count) && _buffer.indexOf("OK") == -1);
	{
		if (_buffer.indexOf("OK") == -1)
		{
			Serial.print(".");
			initilize = false;
			return false;
		}
		else
		{
			Serial.println("AT command Success");
			initilize = true;
			Serial.println("GSM init main: " + String(initilize));
			return true;
		}
	}
}

bool QuectelEC200module::enableECHO()
{
	_Serial->flush();
	int count = 0;
	do
	{
		_Serial->print(F("ATE1\r\n"));

		_buffer = _readSerial(10);
		count++;
		delay(RetryDelay2);
	} while ((count < NumofRetry && count < MAX_Count) && _buffer.indexOf("OK") == -1);
	{
		if ((_buffer.indexOf("OK")) == -1)
		{
			return false;
		}
		else
		{
			return true;
		}
	}
}

bool QuectelEC200module::initilizeModule()
{
	if (!SPIFFS.begin(true))
	{
		Serial.println("SPIFFS Mount Failed");
	}
	uint64_t timeOld = millis();
	while ((millis() < timeOld + SET_AT_TIMEOUT) && !SetAT())
		;

	if (configureModule())
	{
		Serial.println("Configuration Successfull");
		return true;
	}
	else
	{
		Serial.println("Configuration Failed");
		return false;
	}
}

bool QuectelEC200module::configureModule()
{
	uint8_t flag1;
	uint8_t flag2;
	uint8_t flag3;
	uint8_t flag4;
	uint8_t flag5;
	int count = 0;
	_Serial->flush();
	do
	{
		_Serial->print(F("AT+CMEE=2\r\n"));
		_buffer = _readSerial(2000);
		count++;
		delay(RetryDelay);
	}

	while ((count < NumofRetry && count < MAX_Count) && _buffer.indexOf("OK") == -1);
	{
		if (_buffer.indexOf("OK") == -1)
		{
			flag1 = 0;
		}
		else
		{
			flag1 = 1;
		}
	}
	count = 0;
	_Serial->flush();
	do
	{
		_Serial->print(F("AT+CREG=2\r\n"));
		_buffer = _readSerial(2000);
		count++;
		delay(RetryDelay);
	}

	while ((count < NumofRetry && count < MAX_Count) && _buffer.indexOf("OK") == -1);
	{
		if (_buffer.indexOf("OK") == -1)
		{
			flag2 = 0;
		}
		else
		{
			flag2 = 1;
		}
	}
	
	count = 0;
	_Serial->flush();
	do
	{
		_Serial->print(F("AT+CFUN=0\r\n"));
		_buffer = _readSerialUntill("OK", 5000);
		count++;
		delay(RetryDelay);
	}

	while ((count < NumofRetry && count < MAX_Count) && _buffer.indexOf("OK") == -1);
	{
		if (_buffer.indexOf("OK") == -1)
		{
			flag3 = 0;
		}
		else
		{
			flag3 = 1;
		}
	}
	// delay(2000);
	count = 0;
	_Serial->flush();
	do
	{
		_Serial->print(F("AT+CFUN=1\r\n"));
		_buffer = _readSerialUntill("OK", 5000);
		count++;
		delay(RetryDelay);
	}

	while ((count < NumofRetry && count < MAX_Count) && _buffer.indexOf("OK") == -1);
	{
		if (_buffer.indexOf("OK") == -1)
		{
			flag4 = 0;
		}
		else
		{
			flag4 = 1;
		}
		
	}
	count = 0;
	_Serial->flush();
	do
	{
		_Serial->print(F("AT+CPIN?\r\n"));
		_buffer = _readSerial(20);
		count++;
		delay(RetryDelay);
	}

	while ((count < NumofRetry && count < MAX_Count) && _buffer.indexOf("READY") == -1);
	{
		if (_buffer.indexOf("READY") == -1)
		{
			flag5 = 0;
		}
		else
		{
			flag5 = 1;
		}
	}
	if (flag1 == 0 || flag2 == 0 || flag3 == 0 || flag4 == 0 || flag5 == 0)
		{
			return false;
		}
		else
		{
			return true;
		}
}

bool QuectelEC200module::checkforNetwork()
{
	int count = 0;
	_Serial->flush();
	do
	{
		_Serial->print(F("AT+COPS?\r\n"));
		_buffer = _readSerial(20);
		count++;
		delay(500);
		if (count > 25)
		{
			Serial.println("\nNetwork Not Found");
			return false;
			// setLTE();
			// hardRestart();w
			// delay(10000);
			// ESP.restart();
		}
	}

	while ((count < 30) && _buffer.indexOf("\"") == -1);
	{
		if (_buffer.indexOf("\"") == -1)
		{
			return false;
		}
		else
		{
			return true;
		}
	}
}

bool QuectelEC200module::resetSettings()
{
	int count = 0;
	_Serial->flush();
	do
	{
		_Serial->print(F("AT&F\r\n"));
		_buffer = _readSerial(10);
		count++;
		delay(RetryDelay);
	}

	while ((count < NumofRetry && count < MAX_Count) && _buffer.indexOf("OK") == -1);
	{
		if (_buffer.indexOf("OK") == -1)
		{
			return false;
		}
		else
			return true;
	}
}

int QuectelEC200module::getRssi()
{
	int count = 0;
	_Serial->flush();
	String Rssi;
	int rssiDbm;
	do
	{
		_Serial->print(F("AT+CSQ\r\n"));
		_buffer = _readSerial(2000);
		count++;
		delay(RetryDelay);
	}

	while ((count < NumofRetry && count < MAX_Count) && _buffer.indexOf("OK") == -1);
	{
		if (_buffer.indexOf("OK") == -1)
		{
			return false;
		}
		else
			Rssi = midString(_buffer, "+CSQ: ", ",");
		if (int(Rssi.toInt()) <= 30 || int(Rssi.toInt()) >= 2)
		{
			rssiDbm = -(109 - ((int(Rssi.toInt()) - 2) * 2));
		}
		else
		{
			rssiDbm = -51;
		}
		return rssiDbm;
	}
}

String QuectelEC200module::getOperater()
{
	int count = 0;
	_Serial->flush();
	String operater;
	do
	{
		_Serial->print(F("AT+COPS?\r\n"));
		_buffer = _readSerial(1000);
		count++;
		delay(RetryDelay);
	}

	while ((count < NumofRetry && count < MAX_Count) && _buffer.indexOf("OK") == -1);
	{
		if (_buffer.indexOf("OK") == -1)
		{
			return "ERROR";
		}
		else
			operater = midString(_buffer, "\"", "\"");
		return operater;
	}
}

String QuectelEC200module::getNetworkType()
{
	int count = 0;
	_Serial->flush();
	String operater;
	do
	{
		_Serial->print(F("AT+QNWINFO\r\n"));
		_buffer = _readSerial(1000);
		count++;
		delay(RetryDelay);
	}

	while ((count < NumofRetry && count < MAX_Count) && _buffer.indexOf("OK") == -1);
	{
		if (_buffer.indexOf("OK") == -1)
		{
			return "ERROR";
		}
		else
			operater = midString(_buffer, "\"", "\"");
		return operater;
	}
}

String QuectelEC200module::getBandInfo()
{
	int count = 0;
	_Serial->flush();
	String band;
	do
	{
		_Serial->print(F("AT+QNWINFO\r\n"));
		_buffer = _readSerial(1000);
		count++;
		delay(RetryDelay);
	}

	while ((count < NumofRetry && count < MAX_Count) && _buffer.indexOf("OK") == -1);
	{
		if (_buffer.indexOf("OK") == -1)
		{
			return "ERROR";
		}
		else
		{
			band = midStringSecond(_buffer, "\",\"", "\",");
			return band;
		}
	}
}

bool QuectelEC200module::setAPN(const char *apn, const char *username, const char *password, int auth)
{
	int count = 0;
	_Serial->flush();
	do
	{
		_Serial->print(F("AT+QICSGP=1,1,\""));
		_Serial->print(apn);
		_Serial->print(F("\",\""));
		_Serial->print(username);
		_Serial->print(F("\",\""));
		_Serial->print(password);
		_Serial->print(F("\","));
		_Serial->print(auth);
		_Serial->print(F("\r\n"));
		_buffer = _readSerial(1000);
		count++;
		delay(RetryDelay2);
	}

	while ((count < NumofRetry && count < MAX_Count) && _buffer.indexOf("OK") == -1);
	{
		if (_buffer.indexOf("OK") == -1)
		{
			return false;
		}
		else
		{
			return true;
		}
	}
}

String QuectelEC200module::getIMEI()
{
	String response = "";
	int count = 0;
	_Serial->flush();
	do
	{
		_Serial->print(F("AT+CGSN\r\n"));
		_buffer = _readSerial(1000);
		count++;
		delay(RetryDelay2);
	}

	while ((count < NumofRetry && count < MAX_Count) && _buffer.indexOf("OK") == -1);
	{
		if (_buffer.indexOf("OK") == -1)
		{
			return "ERROR";
		}
		else
		{
			response = midString(_buffer, "AT+CGSN\r\r\n", "\r\n\r\nOK");
			return response;
		}
	}
}

String QuectelEC200module::getModelNumber()
{
	String response = "";
	int count = 0;
	_Serial->flush();
	do
	{
		_Serial->print(F("AT+CGMM\r\n"));
		_buffer = _readSerial(1000);
		count++;
		delay(RetryDelay2);
	}

	while ((count < NumofRetry && count < MAX_Count) && _buffer.indexOf("OK") == -1);
	{
		if (_buffer.indexOf("OK") == -1)
		{
			return "ERROR";
		}
		else
		{
			response = midString(_buffer, "AT+CGMM\r\r\n", "\r\n\r\nOK");
			return response;
		}
	}
}

String QuectelEC200module::getSerialNumber()
{
	String response = "";
	int count = 0;
	_Serial->flush();
	do
	{
		_Serial->print(F("AT+EGMR=0,5\r\n"));
		_buffer = _readSerial(1000);
		count++;
		delay(RetryDelay2);
	}

	while ((count < NumofRetry && count < MAX_Count) && _buffer.indexOf("OK") == -1);
	{
		if (_buffer.indexOf("OK") == -1)
		{
			return "ERROR";
		}
		else
		{
			response = midString(_buffer, "+EGMR: \"", "\"\r\n\r\nOK");
			return response;
		}
	}
}

String QuectelEC200module::getManufacturer()
{
	String response = "";
	int count = 0;
	_Serial->flush();
	do
	{
		_Serial->print(F("AT+GMI\r\n"));
		_buffer = _readSerial(1500);
		count++;
		delay(RetryDelay2);
	}

	while ((count < NumofRetry && count < MAX_Count) && _buffer.indexOf("OK") == -1);
	{
		if (_buffer.indexOf("OK") == -1)
		{
			return "ERROR";
		}
		else
		{
			response = midString(_buffer, "AT+GMI\r\r\n", "\r\n\r\nOK");
			return response;
		}
	}
}

String QuectelEC200module::getSimNumber()
{
	String response = "";
	int count = 0;
	_Serial->flush();
	do
	{
		_Serial->print(F("AT+CCID\r\n"));
		_buffer = _readSerial(1000);
		count++;
		delay(RetryDelay2);
	}

	while ((count < NumofRetry && count < MAX_Count) && _buffer.indexOf("OK") == -1);
	{
		if (_buffer.indexOf("OK") == -1)
		{
			return "ERROR";
		}
		else
		{
			response = midString(_buffer, "+CCID:", "\r\n\r\nOK");
			return response;
		}
	}
}

bool QuectelEC200module::simPresence()
{
	String response = "";
	int count = 0;
	_Serial->flush();
	do
	{
		_Serial->print(F("AT+QSIMSTAT?\r\n"));
		_buffer = _readSerial(1000);
		count++;
		delay(RetryDelay2);
	}

	while ((count < NumofRetry && count < MAX_Count) && _buffer.indexOf("OK") == -1);
	{
		if (_buffer.indexOf("OK") == -1)
		{
			return false;
		}
		else
		{
			response = midString(_buffer, "+QSIMSTAT: 0,", "\r\n\r\nOK");
			if (response.indexOf("0") == -1)
			{
				return true;
			}
			else
			{
				return false;
			}
		}
	}
}

void QuectelEC200module::getModuleInfo()
{
	manufacturarer = getManufacturer();
	model = getModelNumber();
	imei = getIMEI();
	serialNumber = getSerialNumber();
	Serial.println("Manufacturer : " + manufacturarer);
	Serial.println("Model        : " + model);
	Serial.println("IMEI         : " + imei);
	Serial.println("S/N          : " + serialNumber);
}

void QuectelEC200module::getSimInfo()
{
	if (simPresence())
	{
		Serial.println("SIM CARD FOUND");
		operater = getOperater();
		techonology = getNetworkType();
		band = getBandInfo();
		rssi = getRssi();
		simNum = getSimNumber();

		Serial.println("Operater   : " + operater);
		Serial.println("Technology : " + techonology);
		Serial.println("Band       : " + band);
		Serial.println("RSSI       : " + String(rssi) + " dBm");
		Serial.println("SIM NO     : " + simNum);
	}
	else
	{
		Serial.println("SIM CARD NOT FOUND");
	}
}

String QuectelEC200module::getCurrentTime(String url)
{

	int count = 0;
	_Serial->flush();
	do
	{
		_Serial->print(F("AT+QNTP=1,\""));
		_Serial->print(url);
		_Serial->print("\"\r\n");
		_buffer = _readSerial(1000);
		count++;
		delay(RetryDelay2);
	}

	while ((count < NumofRetry && count < MAX_Count) && _buffer.indexOf("+QNTP:") == -1);
	{
		if (_buffer.indexOf("+QNTP:") == -1)
		{
			String utcTime = "1970-01-01T00:00:00+00:00";
			return utcTime;
		}
		else
		{
			String fullTime = midString(_buffer, "+QNTP: 0,\"", "\"");
			fullTime.replace("/", "-");
			fullTime.replace(",", "T");
			Serial.println(fullTime);
			return fullTime;
		}
	}
}

bool QuectelEC200module::connectNetwork()
{
	int count = 0;
	_Serial->flush();
	int flag = false;
	do
	{
		_Serial->print(F("AT+QIACT=1\r\n"));
		_buffer = _readSerial(2000);
		count++;
		delay(RetryDelay2);
	}

	while ((count < NumofRetry && count < MAX_Count) && _buffer.indexOf("OK") == -1);
	{
		if (_buffer.indexOf("OK") == -1)
		{
			flag = false;
		}
		else
		{
			flag = true;
		}
	}
	String cellularIP = getIPAddress();
	Serial.println("Cellular IPAddress : " + cellularIP);
	if (cellularIP == "0.0.0.0")
	{
		flag = false;
	}
	else
	{
		flag = true;
	}
	return flag;
}

String QuectelEC200module::getIPAddress()
{
	int count = 0;
	_Serial->flush();
	do
	{
		_Serial->print(F("AT+QIACT?\r\n"));
		_buffer = _readSerial(5000);
		count++;
		delay(RetryDelay2);
	}

	while ((count < NumofRetry && count < MAX_Count) && _buffer.indexOf("+QIACT:") == -1);
	{
		if (_buffer.indexOf("+QIACT:") == -1)
		{
			return "0.0.0.0";
		}
		else
		{
			return midString(_buffer, "\"", "\"");
		}
	}
}

bool QuectelEC200module::disConnectNetwork()
{
	int count = 0;
	_Serial->flush();
	do
	{
		_Serial->print(F("AT+QIDEACT=1\r\n"));
		_buffer = _readSerial(1000);
		count++;
		delay(RetryDelay2);
	}

	while ((count < NumofRetry && count < MAX_Count) && _buffer.indexOf("OK") == -1);
	{
		if (_buffer.indexOf("OK") == -1)
		{
			return false;
		}
		else
		{
			return true;
		}
	}
}

// bool QuectelEC200module::terminateHTTP()
// {
// 	int count = 0;
// 	do
// 	{
// 		_Serial->print(F("AT+QHTTPSTOP\r\n"));

// 		_buffer = _readSerial(200);
// 		count++;
// 		delay(RetryDelay);
// 	} while ((count < NumofRetry && count < MAX_Count) && _buffer.indexOf("OK") == -1);
// 	{
// 		if ((_buffer.indexOf("OK")) == -1)
// 		{
// 			return false;
// 		}
// 		else
// 		{
// 			return true;
// 		}
// 	}
// }

/*
0 - application/x-www-form-urlencoded
1 - text/plain
2 - application/octet-stream
3 - multipart/form-data
4 - application/json
5 - 5image/jpeg
*/
bool QuectelEC200module::httpContentType(uint8_t type)
{

	if (type > 5)
	{
		Serial.println("Invalid Content type");
	}
	else
	{
		int count = 0;
		_Serial->flush();
		do
		{
			_Serial->print(F("AT+QHTTPCFG=\"contenttype\","));
			_Serial->print(type);
			_Serial->print(F("\r\n"));
			_buffer = _readSerial(1000);
			count++;
			delay(RetryDelay);
		}

		while ((count < NumofRetry && count < MAX_Count) && _buffer.indexOf("OK") == -1);
		{
			if (_buffer.indexOf("OK") == -1)
			{
				return false;
			}
			else
			{
				return true;
			}
		}
	}
}

bool QuectelEC200module::initateHTTP()
{
	uint8_t flag1;
	uint8_t flag2;
	uint8_t flag3;
	int count = 0;

	do
	{
		_Serial->flush();
		_Serial->print(F("AT+QSSLCFG=\"SNI\",1,1\r\n"));
		_buffer = _readSerial(1000);
		count++;
		delay(RetryDelay);
	}

	while ((count < NumofRetry && count < MAX_Count) && _buffer.indexOf("OK") == -1);
	{
		if (_buffer.indexOf("OK") == -1)
		{
			flag1 = 0;
		}
		else
		{
			flag1 = 1;
		}
	}
	count = 0;
	do
	{
		_Serial->flush();
		_Serial->print(F("AT+QSSLCFG=\"ciphersuite\",1,0xFFFF\r\n"));
		_buffer = _readSerial(1000);
		count++;
		delay(RetryDelay);
	}

	while ((count < NumofRetry && count < MAX_Count) && _buffer.indexOf("OK") == -1);
	{
		if (_buffer.indexOf("OK") == -1)
		{
			flag2 = 0;
		}
		else
		{
			flag2 = 1;
		}
	}
	
	
	if (flag1 == 0 || flag2 == 0 || flag3 == 0)
	{
		return false;
	}
	else
	{
		return true;
	}
}

bool QuectelEC200module::ping(String url)
{
	int count = 0;
	_Serial->flush();
	delay(100);
	do
	{
		_Serial->flush();
		_Serial->print(F("AT+QPING=1,\""));
		_Serial->print(url);
		_Serial->print(F("\",10,5\r\n"));
		_buffer = _readSerialUntill("+QPING: 0,5,5", 10000);
		count++;
		delay(5000);
	}

	while ((count < NumofRetry && count < MAX_Count) && _buffer.indexOf("+QPING: 0,5,5") == -1);
	{
		if (_buffer.indexOf("+QPING: 0,5,5") == -1)
		{
			return false;
		}
		else
		{
			return true;
		}
	}
}

size_t QuectelEC200module::getFileSize(char *filename)
{
	int count = 0;
	_Serial->flush();
	do
	{
		_Serial->print(F("AT+QFLST\r\n"));
		_buffer = _readSerial(2000);
		count++;
		delay(RetryDelay);
	}

	while ((count < NumofRetry && count < MAX_Count) && _buffer.indexOf("temp") == -1);
	{
		if (_buffer.indexOf("temp") == -1)
		{
			return false;
		}
		else
		{
			return midString(_buffer, "temp\",", "\r\nOK").toInt();
		}
	}
}

bool QuectelEC200module::deleteFile(String filename)
{
	int count = 0;
	_Serial->flush();
	do
	{
		_Serial->print(F("AT+QFDEL=\""));
		_Serial->print(filename);
		_Serial->print(F("\"\r\n"));
		_buffer = _readSerial(1000);
		count++;
		delay(RetryDelay);
	}

	while ((count < NumofRetry && count < MAX_Count) && _buffer.indexOf("OK") == -1);
	{
		if (_buffer.indexOf("OK") == -1)
		{
			return false;
		}
		else
		{
			return true;
		}
	}
}

bool QuectelEC200module::addHeader(String name, String value)
{
	int count = 0;
	// _Serial->flush();
	do
	{
		_Serial->flush();
		_Serial->print(F("AT+QHTTPCFG=\"custom_header\",\""));
		_Serial->print(name + ":" + value);
		_Serial->print(F("\"\r\n"));
		_buffer = _readSerial(1000);
		count++;
		delay(RetryDelay);
	}

	while ((count < NumofRetry && count < MAX_Count) && _buffer.indexOf("OK") == -1);
	{
		if (_buffer.indexOf("OK") == -1)
		{
			return false;
		}
		else
		{
			return true;
		}
	}
}

int QuectelEC200module::updateESP(char *md5Checksum)
{
	uint8_t fileHandler = 0;
	uint16_t chunkSize = DOWNLOAD_CHUNK_SIZE;
	uint32_t fileSize = getFileSize("temp");
	uint16_t downloadParts = fileSize / chunkSize;
	uint16_t lastPacket = fileSize % chunkSize;
	int seeksize = 0;
	int count = 0;
	do
	{
		_Serial->print(F("AT+QFOPEN=\"temp\"\r\n"));
		_buffer = _readSerial(100);
		count++;
		delay(RetryDelay);
	}

	while ((count < NumofRetry && count < MAX_Count) && _buffer.indexOf("+QFOPEN:") == -1);
	{
		if (_buffer.indexOf("+QFOPEN:") == -1)
		{
			Serial.println("File Open Failed!");
			return false;
		}
		else
		{
			fileHandler = midString(_buffer, "+QFOPEN:", "\r\nOK").toInt();
		}
	}
	if (Update.begin(fileSize, U_FLASH))
	{
		Update.setMD5(md5Checksum);
		for (int i = 0; i <= downloadParts; i++)
		{

			count = 0;
			if (i == 0)
			{
				seeksize = 0;
			}
			else
			{
				seeksize = seeksize + chunkSize;
				Serial.print("Download Percentage : ");
				Serial.print((seeksize * 100) / fileSize);
				Serial.println("%");
			}

			if (i == downloadParts)
			{
				chunkSize = lastPacket;
			}
			do
			{
				_Serial->print(F("AT+QFSEEK="));
				_Serial->print(fileHandler);
				_Serial->print(F(","));
				_Serial->print(seeksize);
				_Serial->print(F(","));
				if (i == 0)
				{
					_Serial->print(F("0\r\n"));
				}
				else
				{
					_Serial->print(F("0\r\n"));
				}
				_buffer = _readSerial(100);
				count++;
				delay(RetryDelay);
			}

			while ((count < NumofRetry && count < MAX_Count) && _buffer.indexOf("OK") == -1);
			{
				if (_buffer.indexOf("OK") == -1)
				{
					Serial.println("Seek Failed");
				}
			}

			count = 0;
			do
			{
				_Serial->print(F("AT+QFREAD="));
				_Serial->print(fileHandler);
				_Serial->print(F(","));
				_Serial->print(chunkSize);
				_Serial->print(F("\r\n"));
				_buffer = storeFile(chunkSize, 100);
				count++;
				delay(50);
			}

			while ((count < NumofRetry && count < MAX_Count) && _buffer.indexOf("OK") == -1);
			{
				if (_buffer.indexOf("OK") == -1)
				{
					Serial.println("Read Failed");
				}
			}
		}
	}
	count = 0;
	do
	{
		_Serial->print(F("AT+QFCLOSE="));
		_Serial->print(fileHandler);
		_Serial->print(F("\r\n"));
		_buffer = _readSerial(500);
		count++;
		delay(RetryDelay);
	}

	while ((count < NumofRetry && count < MAX_Count) && _buffer.indexOf("OK") == -1);
	{
		if (_buffer.indexOf("OK") == -1)
		{
			Serial.println("File Close Failed");
		}
		else
		{
			// deleteFile("temp");
			if (Update.end())
			{
				// Serial.println("Flashing ... done!");
				// delay(100);
				// Serial.println("Rebooting ESP");
				// ESP.restart();
				return true;
			}
			else
			{
				Serial.println("Flashing Failed");
				return -1;
			}
		}
	}
}

int QuectelEC200module::readFile(const char *filename)
{
	uint8_t fileHandler = 0;
	uint16_t chunkSize = DOWNLOAD_CHUNK_SIZE;
	uint32_t fileSize = getFileSize("temp");
	uint16_t downloadParts = fileSize / chunkSize;
	uint16_t lastPacket = fileSize % chunkSize;
	int seeksize = 0;
	int count = 0;
	do
	{
		_Serial->print(F("AT+QFOPEN=\"temp\"\r\n"));
		_buffer = _readSerial(100);
		count++;
		delay(RetryDelay);
	}

	while ((count < NumofRetry && count < MAX_Count) && _buffer.indexOf("+QFOPEN:") == -1);
	{
		if (_buffer.indexOf("+QFOPEN:") == -1)
		{
			Serial.println("File Open Failed!");
			return false;
		}
		else
		{
			fileHandler = midString(_buffer, "+QFOPEN:", "\r\nOK").toInt();
		}
	}
	File f = SPIFFS.open(filename, "w");
	for (int i = 0; i <= downloadParts; i++)
	{

		count = 0;
		if (i == 0)
		{
			seeksize = 0;
		}
		else
		{
			seeksize = seeksize + chunkSize;
			Serial.print("Download Percentage : ");
			Serial.print((seeksize * 100) / fileSize);
			Serial.println("%");
		}

		if (i == downloadParts)
		{
			chunkSize = lastPacket;
		}
		do
		{
			_Serial->print(F("AT+QFSEEK="));
			_Serial->print(fileHandler);
			_Serial->print(F(","));
			_Serial->print(seeksize);
			_Serial->print(F(","));
			if (i == 0)
			{
				_Serial->print(F("0\r\n"));
			}
			else
			{
				_Serial->print(F("0\r\n"));
			}
			_buffer = _readSerial(100);
			count++;
			delay(RetryDelay);
		}

		while ((count < NumofRetry && count < MAX_Count) && _buffer.indexOf("OK") == -1);
		{
			if (_buffer.indexOf("OK") == -1)
			{
				Serial.println("Seek Failed");
			}
		}

		count = 0;
		do
		{
			_Serial->print(F("AT+QFREAD="));
			_Serial->print(fileHandler);
			_Serial->print(F(","));
			_Serial->print(chunkSize);
			_Serial->print(F("\r\n"));
			_buffer = storeFile(f, chunkSize, 100);
			count++;
			delay(RetryDelay);
		}

		while ((count < NumofRetry && count < MAX_Count) && _buffer.indexOf("OK") == -1);
		{
			if (_buffer.indexOf("OK") == -1)
			{
				Serial.println("Read Failed");
			}
		}
	}
	f.close();
	count = 0;
	do
	{
		_Serial->print(F("AT+QFCLOSE="));
		_Serial->print(fileHandler);
		_Serial->print(F("\r\n"));
		_buffer = _readSerial(500);
		count++;
		delay(RetryDelay);
	}

	while ((count < NumofRetry && count < MAX_Count) && _buffer.indexOf("OK") == -1);
	{
		if (_buffer.indexOf("OK") == -1)
		{
			Serial.println("File Close Failed");
		}
		else
		{
			deleteFile("temp");
			Serial.println("Download Successfull");
			Serial.println(filename);
			Serial.println("Downaload CheckSum : " + getMD5checksum(filename));
			return true;
		}
	}
}

uint16_t QuectelEC200module::PostHTTP(String URL, String message, int type)
{

	// delay(100);
	uint8_t flag1;
	uint8_t flag2;
	uint8_t flag3;
	int count = 0;
	initateHTTP();
	httpContentType(type);
	do
	{
		_Serial->flush();
		delay(RetryDelay);
		_Serial->print(F("AT+QHTTPURL="));
		_Serial->print(URL.length());
		_Serial->print(F("\r\n"));
		_buffer = _readSerialUntill("CONNECT", 5000);
		count++;
	}

	while ((count < NumofRetry && count < MAX_Count) && _buffer.indexOf("CONNECT") == -1);
	{
		if (_buffer.indexOf("CONNECT") == -1)
		{
			flag1 = 0;
		}
		else
		{
			flag1 = 1;
		}
	}
	delay(1000);
	count = 0;
	do
	{
		_Serial->flush();
		delay(RetryDelay);
		_Serial->print(URL+ String("\r\n"));
		_buffer = _readSerial(100);
		count++;
	}

	while ((count < NumofRetry && count < MAX_Count) && _buffer.indexOf("OK") == -1);
	{
		if (_buffer.indexOf("OK") == -1)
		{
			flag2 = 0;
		}
		else
		{
			flag2 = 1;
		}
	}
	// delay(100);
	count = 0;
	do
	{
		_Serial->flush();
		_Serial->print(F("AT+QHTTPPOST="));
		_Serial->print(message.length());
		_Serial->print(F(",15,15"));
		_Serial->print(F("\r\n"));
		_buffer = _readSerialUntill("CONNECT", 5000);
		// _buffer = _readSerial(5000);
		count++;
	} while ((count < NumofRetry && count < MAX_Count) && _buffer.indexOf("CONNECT") == -1);
	{
		if (_buffer.indexOf("CONNECT") == -1)
		{
			flag3 = 0;
		}
		else
		{
			flag3 = 1;
		}
	}
	// delay(100);
	count = 0;
	do
	{
		_Serial->flush();
		delay(RetryDelay);
		_Serial->print(message + String("\r\n"));
		_buffer = _readSerialUntill("+QHTTPPOST:", 5000);
		// _buffer = _readSerial(5000);
		count++;

	} while ((count < NumofRetry && count < MAX_Count) && _buffer.indexOf("+QHTTPPOST:") == -1);
	{
		if (_buffer.indexOf("+QHTTPPOST:") == -1)
		{
			return 404;
		}
		else
		{
			String response = midString(_buffer, ",", ",");
			return (uint16_t)response.toInt();
		}
	}
	// delay(100);
}

uint16_t QuectelEC200module::GetHTTP(String URL)
{
	uint8_t flag1;
	uint8_t flag2;
	int count = 0;
	initateHTTP();
	// delay(100);
	do
	{
		_Serial->flush();
		delay(RetryDelay);
		_Serial->print(F("AT+QHTTPURL="));
		_Serial->print(URL.length());
		_Serial->print(F("\r\n"));
		_buffer = _readSerialUntill("CONNECT", 5000);
		count++;
	}

	while ((count < NumofRetry && count < MAX_Count) && _buffer.indexOf("CONNECT") == -1);
	{
		if (_buffer.indexOf("CONNECT") == -1)
		{
			flag1 = 0;
		}
		else
		{
			flag1 = 1;
		}
	}
	count = 0;
	do
	{
		_Serial->flush();
		delay(RetryDelay);
		_Serial->print(URL);
		_Serial->print(F("\r\n"));
		_buffer = _readSerial(100);
		count++;
	}

	while ((count < NumofRetry && count < MAX_Count) && _buffer.indexOf("OK") == -1);
	{
		if (_buffer.indexOf("OK") == -1)
		{
			flag2 = 0;
		}
		else
		{
			flag2 = 1;
		}
	}
	count = 0;
	do
	{
		_Serial->flush();
		_Serial->print(F("AT+QHTTPGET=80\r\n"));
		_buffer = _readSerialUntill("+QHTTPGET:", 10000);
		// _buffer = _readSerial(1000);
		count++;
	} while ((count < NumofRetry && count < MAX_Count) && _buffer.indexOf("+QHTTPGET:") == -1);
	{
		if (_buffer.indexOf("+QHTTPGET:") == -1)
		{
			return 404;
		}
		else
		{
			String response = midString(_buffer, ",", ",");
			return (uint16_t)response.toInt();
		}
	}
	// delay(100);
}

int QuectelEC200module::downloadUpdate(const char *URL, char *md5Checksum)
{
	uint8_t flag1;
	uint8_t flag2;
	uint8_t flag3;
	uint8_t flag4;
	size_t size = 0;
	int count = 0;
	initateHTTP();
	// httpContentType(2);
	// delay(100);

	count = 0;
	_Serial->flush();
	do
	{
		_Serial->flush();
		delay(RetryDelay);
		_Serial->print(F("AT+QHTTPURL="));
		_Serial->print(sizeof(URL));
		_Serial->print(F("\r\n"));
		_buffer = _readSerialUntill("CONNECT", 5000);
		count++;
	}

	while ((count < NumofRetry && count < MAX_Count) && _buffer.indexOf("CONNECT") == -1);
	{
		if (_buffer.indexOf("CONNECT") == -1)
		{
			flag1 = 0;
		}
		else
		{
			flag1 = 1;
		}
	}
	count = 0;
	do
	{
		_Serial->flush();
		delay(RetryDelay);
		_Serial->print(URL);
		_Serial->print(F("\r\n"));
		_buffer = _readSerial(100);
		count++;
	}

	while ((count < NumofRetry && count < MAX_Count) && _buffer.indexOf("OK") == -1);
	{
		if (_buffer.indexOf("OK") == -1)
		{
			flag2 = 0;
		}
		else
		{
			flag2 = 1;
		}
	}
	// delay(100);
	count = 0;
	_Serial->flush();
	do
	{
		_Serial->print(F("AT+QHTTPGET="));
		// _Serial->print(strlen(message));
		_Serial->print(F("100"));
		_Serial->print(F("\r\n"));
		// _buffer = _readSerial(2000);
		_buffer = _readSerialUntill("200", 3000);
		count++;
	} while ((count < NumofRetry && count < MAX_Count) && _buffer.indexOf("200") == -1);
	{
		if (_buffer.indexOf("200") == -1)
		{
			flag3 = 0;
		}
		else
		{
			flag3 = 1;
		}
	}
	count = 0;
	_Serial->flush();
	do
	{
		_Serial->print(F("AT+QHTTPREADFILE=\"temp\",300\r\n"));
		_buffer = _readSerialUntill("+QHTTPREADFILE: 0\r\n", 120 * 1000);
		count++;
	} while ((count < NumofRetry && count < MAX_Count) && _buffer.indexOf("+QHTTPREADFILE: 0\r\n") == -1);
	{
		if (_buffer.indexOf("+QHTTPREADFILE: 0\r\n") == -1)
		{
			flag4 = 0;
		}
		else
		{
			flag4 = 1;
			if (updateESP(md5Checksum) == -1)
			{
				flag4 = -1;
			}
		}
	}
	// delay(100);

	if (flag1 == 1 || flag2 == 1 || flag3 == 1 || flag4 == 1)
	{
		return true;
	}
	else
	{
		return false;
	}
}

bool QuectelEC200module::downloadFile(const char *URL, const char *filename)
{
	uint8_t flag1;
	uint8_t flag2;
	uint8_t flag3;
	uint8_t flag4;
	size_t size = 0;
	int count = 0;
	initateHTTP();
	httpContentType(2);
	_Serial->flush();
	count = 0;
	do
	{
		_Serial->flush();
		delay(RetryDelay);
		_Serial->print(F("AT+QHTTPURL="));
		_Serial->print(sizeof(URL));
		_Serial->print(F("\r\n"));
		_buffer = _readSerialUntill("CONNECT", 5000);
		count++;
	}

	while ((count < NumofRetry && count < MAX_Count) && _buffer.indexOf("CONNECT") == -1);
	{
		if (_buffer.indexOf("CONNECT") == -1)
		{
			flag1 = 0;
		}
		else
		{
			flag1 = 1;
		}
	}
	count = 0;
	do
	{
		_Serial->flush();
		delay(RetryDelay);
		_Serial->print(URL);
		_Serial->print(F("\r\n"));
		_buffer = _readSerial(100);
		count++;
	}

	while ((count < NumofRetry && count < MAX_Count) && _buffer.indexOf("OK") == -1);
	{
		if (_buffer.indexOf("OK") == -1)
		{
			flag2 = 0;
		}
		else
		{
			flag2 = 1;
		}
	}
	// delay(100);
	count = 0;
	_Serial->flush();
	do
	{
		_Serial->print(F("AT+QHTTPGET="));
		// _Serial->print(strlen(message));
		_Serial->print(F("100"));
		_Serial->print(F("\r\n"));
		// _buffer = _readSerial(2000);
		_buffer = _readSerialUntill("200", 3000);
		count++;
	} while ((count < NumofRetry && count < MAX_Count) && _buffer.indexOf("200") == -1);
	{
		if (_buffer.indexOf("200") == -1)
		{
			flag3 = 0;
		}
		else
		{
			flag3 = 1;
		}
	}
	count = 0;
	_Serial->flush();
	do
	{
		_Serial->print(F("AT+QHTTPREADFILE=\"temp\",300\r\n"));
		_buffer = _readSerialUntill("+QHTTPREADFILE: 0\r\n", 120 * 1000);
		count++;
	} while ((count < NumofRetry && count < MAX_Count) && _buffer.indexOf("+QHTTPREADFILE: 0\r\n") == -1);
	{
		if (_buffer.indexOf("+QHTTPREADFILE: 0\r\n") == -1)
		{
			flag4 = 0;
		}
		else
		{
			flag4 = 1;
			readFile(filename);
		}
	}
	// delay(100);

	if (flag1 == 1 || flag2 == 1 || flag3 == 1 || flag4 == 1)
	{
		return true;
	}
	else
	{
		return false;
	}
}

uint16_t QuectelEC200module::PutHTTP(String URL, String message, int type)
{
	uint8_t flag1;
	uint8_t flag2;
	uint8_t flag3;
	int count = 0;
	initateHTTP();
	httpContentType(type);
	count = 0;
	do
	{
		_Serial->flush();
		delay(RetryDelay);
		_Serial->print(F("AT+QHTTPURL="));
		_Serial->print(URL.length());
		_Serial->print(F("\r\n"));
		_buffer = _readSerialUntill("CONNECT", 5000);
		count++;
	}

	while ((count < NumofRetry && count < MAX_Count) && _buffer.indexOf("CONNECT") == -1);
	{
		if (_buffer.indexOf("CONNECT") == -1)
		{
			flag1 = 0;
		}
		else
		{
			flag1 = 1;
		}
	}
	count = 0;
	do
	{
		_Serial->flush();
		delay(RetryDelay);
		_Serial->print(URL);
		_Serial->print(F("\r\n"));
		_buffer = _readSerial(100);
		count++;
	}

	while ((count < NumofRetry && count < MAX_Count) && _buffer.indexOf("OK") == -1);
	{
		if (_buffer.indexOf("OK") == -1)
		{
			flag2 = 0;
		}
		else
		{
			flag2 = 1;
		}
	}
	// delay(100);
	count = 0;
	do
	{
		_Serial->flush();
		_Serial->print(F("AT+QHTTPPUT="));
		_Serial->print(message.length());
		_Serial->print(F(",15,15"));
		_Serial->print(F("\r\n"));
		_buffer = _readSerial(1000);
		count++;
	} while ((count < NumofRetry && count < MAX_Count) && _buffer.indexOf("CONNECT") == -1);
	{
		if (_buffer.indexOf("CONNECT") == -1)
		{
			flag3 = 0;
		}
		else
		{
			flag3 = 1;
		}
	}
	// delay(100);
	count = 0;
	do
	{
		_Serial->flush();
		delay(RetryDelay);
		_Serial->print(message + String("\r\n"));
		_buffer = _readSerialUntill("+QHTTPPUT:", 3000);
		count++;

	} while ((count < NumofRetry && count < MAX_Count) && _buffer.indexOf("+QHTTPPUT:") == -1);
	{
		if (_buffer.indexOf("+QHTTPPUT:") == -1)
		{
			return 404;
		}
		else
		{
			String response = midString(_buffer, ",", ",");
			return (uint16_t)response.toInt();
		}
	}
	// delay(100);
}

String QuectelEC200module::HTTPread()
{
	int count = 0;
	String response = "";
	do
	{
		_Serial->flush();
		_Serial->print(F("AT+QHTTPREAD"));
		_Serial->print(F("\r\n"));
		_buffer = _readSerialString(1000);
		count++;
		delay(RetryDelay);
	} while ((count < NumofRetry && count < MAX_Count) && _buffer.indexOf("OK") == -1);
	{
		if (_buffer.indexOf("OK") == -1)
		{
			response = "";
		}
		else
		{
			response = midString(_buffer, "CONNECT\r\n", "\r\nOK");
		}
	}
	return response;
}

String QuectelEC200module::SerialRead()
{
	_timeout = 0;
	while (!_Serial->available() && _timeout < 12000)
	{
		delay(13);
		_timeout++;
	}
	if (_Serial->available())
	{
		return _Serial->readString();
	}
}

//
////
// PRIVATE METHODS
//
//  String QuectelEC200module::_readSerial()
//  {
//  	uint64_t timeOld = millis();
//  	while (!_Serial->available() || !(millis() > timeOld + TIME_OUT_READ_SERIAL))
//  	{
//  	}
//  	String str = "";
//  	if (_Serial->available() > 0)
//  	{
//  		str += (char)_Serial->read();
//  	}
//  	#if ENABLE_DEBUG
//  		Serial.println(str);
//  	#endif
//  	return str;
//  }

// String QuectelEC200module::_readSerial(uint32_t timeout)
// {
// 	_buffer = "";
// 	uint64_t timeOld = millis();
// 	while (!_Serial->available() && !(millis() > timeOld + timeout))
// 	{

// 	}
// 	String str = "";

// 	if (_Serial->available() > 0)
// 	{
// 		str = _Serial->readString();
// 	}
// 	#if ENABLE_DEBUG
// 		Serial.println(str);
// 	#endif
// 	return str;
// }
String QuectelEC200module::_readSerial()
{
	uint64_t timeOld = millis();
	while (!_Serial->available() && !(millis() > timeOld + TIME_OUT_READ_SERIAL))
	{
		delay(35);
	}
	String str = "";
	while (_Serial->available())
	{
		if (_Serial->available() > 0)
		{
			str += (char)_Serial->read();
			delay(1);
		}
	}
#if ENABLE_DEBUG
	Serial.println(str);
#endif
	return str;
}

String QuectelEC200module::_readSerial(uint32_t timeout)
{
	uint64_t timeOld = millis();
	while (!_Serial->available() && !(millis() > timeOld + timeout))
	{
		delay(35);
	}
	String str = "";
	// timeOld = millis();
	while (_Serial->available() /* && (millis() > timeOld + timeout)*/)
	{
		if (_Serial->available() > 0)
		{
			str += (char)_Serial->read();
			delay(1);
		}
	}
#if ENABLE_DEBUG
	Serial.println(str);
#endif
	return str;
}

String QuectelEC200module::_readSerialString(uint32_t timeout)
{
	uint64_t timeOld = millis();
	while (!_Serial->available() && !(millis() > timeOld + timeout))
	{
	}
	String str = "";

	if (_Serial->available() > 0)
	{
		str = _Serial->readString();
	}
#if ENABLE_DEBUG
	Serial.println(str);
#endif
	return str;
}

String QuectelEC200module::_readSerialUntill(String buff, uint32_t timeout)
{
	_buffer = "";
	uint64_t timeOld = millis();
	String str = "";
	Serial.print("\r\n");
	while (_Serial->available() > 0 || !(millis() > timeOld + timeout))
	{
		Serial.print(".");
		str = _Serial->readString();
#if ENABLE_DEBUG
		Serial.println(str);
#endif
		if (str.indexOf(buff) == -1)
		{
		}
		else
		{
			break;
		}
	}
#if ENABLE_DEBUG
	Serial.println(str);
#endif
	Serial.print("\r\n");
	return str;
}

String QuectelEC200module::midString(String str, String start, String finish)
{
	int locStart = str.indexOf(start);
	if (locStart == -1)
		return "";
	locStart += start.length();
	int locFinish = str.indexOf(finish, locStart);
	if (locFinish == -1)
		return "";
	return str.substring(locStart, locFinish);
}

String QuectelEC200module::midStringSecond(String str, String start, String finish)
{
	int locStart = str.indexOf(start);
	if (locStart == -1)
		return "";
	locStart = locStart + 1;
	if (locStart == -1)
		return "";
	int temp = str.indexOf(start, locStart);
	locStart = temp + start.length();
	if (locStart == -1)
		return "";
	int locFinish = str.indexOf(finish, locStart);
	if (locFinish == -1)
		return "";
	return (str.substring(locStart, locFinish));
}

String QuectelEC200module::file_md5(File &f)
{
	if (!f)
	{
		return String();
	}

	if (f.seek(0, SeekSet))
	{

		MD5Builder md5;
		md5.begin();
		md5.addStream(f, f.size());
		md5.calculate();
		return md5.toString();
	}
	return String();
}

String QuectelEC200module::getMD5checksum(const char *path)
{
	File c = SPIFFS.open(path);
	String md5str = file_md5(c);
	return md5str;
}

String QuectelEC200module::storeFile(uint16_t rxBuffersize, uint32_t timeout)
{
	uint64_t timeOld = millis();
	String str = "";
	uint8_t disB = 0;
	uint8_t disL = 6;
	uint8_t addB = 0;
	if (numberOfDigits(rxBuffersize) == 5)
	{
		disB = 34;
	}
	else if (numberOfDigits(rxBuffersize) == 4)
	{
		disB = 32;
	}
	else if (numberOfDigits(rxBuffersize) == 3)
	{
		disB = 30;
	}
	else if (numberOfDigits(rxBuffersize) == 2)
	{
		disB = 28;
	}
	else
	{
		disB = 26;
	}
	addB = disB + disL + 1;
	_Serial->setRxBufferSize(rxBuffersize + addB);
	delay(1000);
	size_t size;

	int count = 0;
	size = _Serial->available();

#if ENABLE_DEBUG > 1
	Serial.println("Size : " + String(size));
#endif

	while (_Serial->available() || !(millis() > timeOld + timeout))
	{
		if (count > disB && count < (size - disL))
		{
			uint8_t c = _Serial->read();
			Update.write(&c, 1);
		}
		else
		{
			str += (char)_Serial->read();
		}
		if (count > size)
		{
			break;
		}
		count++;
	}
#if ENABLE_DEBUG > 1
	Serial.println(str);
#endif
	return str;
}

String QuectelEC200module::storeFile(File f, uint16_t rxBuffersize, uint32_t timeout)
{
	uint64_t timeOld = millis();
	String str = "";
	uint8_t disB = 0;
	uint8_t disL = 6;
	uint8_t addB = 0;
	if (numberOfDigits(rxBuffersize) == 5)
	{
		disB = 34;
	}
	else if (numberOfDigits(rxBuffersize) == 4)
	{
		disB = 32;
	}
	else if (numberOfDigits(rxBuffersize) == 3)
	{
		disB = 30;
	}
	else if (numberOfDigits(rxBuffersize) == 2)
	{
		disB = 28;
	}
	else
	{
		disB = 26;
	}
	addB = disB + disL + 1;
	_Serial->setRxBufferSize(rxBuffersize + addB);
	delay(1000);
	size_t size;

	int count = 0;
	size = _Serial->available();

#if ENABLE_DEBUG > 1
	Serial.println("Size : " + String(size));
#endif

	while (_Serial->available() || !(millis() > timeOld + timeout))
	{
		if (count > disB && count < (size - disL))
		{
			f.write(_Serial->read());
		}
		else
		{
			str += (char)_Serial->read();
		}
		if (count > size)
		{
			break;
		}
		count++;
	}
#if ENABLE_DEBUG > 1
	Serial.println(str);
#endif
	return str;
}

String QuectelEC200module::convertToString(char *a, int size)
{
	int i;
	String s = "";
	for (i = 0; i < size; i++)
	{
		s = s + a[i];
	}
	return s;
}

int QuectelEC200module::numberOfDigits(uint16_t n)
{
	int count = 0;
	while (n != 0)
	{
		n /= 10; // n = n/10
		++count;
	}
	return count;
}

QuectelEC200module EC200module;
