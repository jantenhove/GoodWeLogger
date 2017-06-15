#pragma once
#include <vector>
#include <SoftwareSerial.h>
#include <ESP8266WiFi.h>
#include "SettingsManager.h"

#define GOODWE_COMMS_ADDRES 0xAB
#define PACKET_TIMEOUT 500			//0.5 sec packet timeout
#define OFFLINE_TIMEOUT 30000		//30 seconds no data -> inverter offline
#define DISCOVERY_INTERVAL 10000	//10 secs between discovery 
#define INFO_INTERVAL 1000			//get inverter info every second

class GoodWeCommunicator
{
public:
	struct GoodweInverterInformation
	{
		char serialNumber[17];		//serial number (ascii) from inverter
		char address;				//address provided by this software
		bool addressConfirmed;		//wether or not the address is confirmed by te inverter
		unsigned long lastSeen;		//when was the inverter last seen? If not seen for 30 seconds the inverter is marked offline. 
		bool isOnline;				//is the inverter online (see above)
		bool isDTSeries;			//is tri phase inverter (get phase 2, 3 info)

		//inverert info from inverter pdf. Updated by the inverter info command
		float vpv1=0.0;
		float vpv2=0.0;
		float ipv1=0.0;
		float ipv2 = 0.0;
		float vac1 = 0.0;
		float vac2 = 0.0;
		float vac3 = 0.0;
		float iac1 = 0.0;
		float iac2 = 0.0;
		float iac3 = 0.0;
		float fac1 = 0.0;
		float fac2 = 0.0;
		float fac3 = 0.0;
		short pac=0;
		short workMode=0;
		float temp = 0.0;
		int errorMessage=0;
		int eTotal=0;
		int hTotal=0;
		float tempFault = 0.0;
		float pv1Fault = 0.0;
		float pv2Fault = 0.0;
		float line1VFault = 0.0;
		float line2VFault = 0.0;
		float line3VFault = 0.0;
		float line1FFault = 0.0;
		float line2FFault = 0.0;
		float line3FFault = 0.0;
		short gcfiFault=0;
		float eDay = 0.0;
	};

	GoodWeCommunicator(SettingsManager * settingsManager, bool debugMode = false);
	void start();
	void stop();
	void handle();

	std::vector<GoodweInverterInformation> getInvertersInfo();
	~GoodWeCommunicator();

private:


	static const int BufferSize = 96;	// largest packet is 67 bytes long. Extra for receiving with sliding window 
	SoftwareSerial * goodweSerial;

	char headerBuffer[7];
	char inputBuffer[BufferSize];
	char outputBuffer[BufferSize];

	bool debugMode;

	unsigned long lastReceived = 0;			//timeout detection
	bool startPacketReceived = false;		//start packet marker
	char lastReceivedByte = 0;				//packet start consist of 2 bytes to test. This holds the previous byte
	int curReceivePtr = 0;					//the ptr in our OutputBuffer when reading
	int numToRead = 0;						//number of bytes to read after the header is read.

	unsigned long lastDiscoverySent = 0;	//discovery needs to be sent every 10 secs. 
	unsigned long lastInfoUpdateSent = 0;	//last info update sent to the registered inverters
	char lastUsedAddress = 0;				//last used address counter. When overflows will only allocate not used

	std::vector<GoodWeCommunicator::GoodweInverterInformation> inverters;

	int sendData(char address, char controlCode, char functionCode, char dataLength, char * data);
	void debugPrintHex(char cnt);
	void sendDiscovery();
	void checkOfflineInverters();
	void checkIncomingData();
	void parseIncomingData(char dataLength);
	void handleRegistration(char * serialNumber, char length);
	void handleRegistrationConfirmation(char address);
	void handleIncomingInformation(char address, char dataLengthh, char * data);
	float bytesToFloat(char * bt, char factor);
	void askAllInvertersForInformation();
	void askInverterForInformation(char address);
	GoodWeCommunicator::GoodweInverterInformation * getInverterInfoByAddress(char address);
	void sendAllocateRegisterAddress(char * serialNumber, char Address);
	void sendRemoveRegistration(char address);
};
