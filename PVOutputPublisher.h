#pragma once
#include <Arduino.h>
#include "TimeLib.h"
#include <ESP8266WiFi.h>
#include "SettingsManager.h"
#include "GoodWeCommunicator.h"
#include "ESP8266HTTPClient.h"
class PVOutputPublisher
{
public:
	PVOutputPublisher(SettingsManager * settingsManager, GoodWeCommunicator *goodWe, bool inDebugMode = false);
	~PVOutputPublisher();

	void start();
	void stop();
	bool canStart();
	bool getIsStarted();
	void sendToPvOutput(GoodWeCommunicator::GoodweInverterInformation info);

	void handle();

private:
	SettingsManager::Settings * pvoutputSettings;
	SettingsManager * pvOutputSettingsManager;
	GoodWeCommunicator * goodweCommunicator;
	bool debugMode;
	unsigned long lastUpdated;
	bool isStarted = false;	 
	unsigned long currentPacSum = 0;
	unsigned int lastPac = 0;
	float lastVoltage = 0;
	double currentVoltageSum = 0;
	float lastTemp;
	double currentTemp = 0;
	double currentTempSum = 0;
	unsigned long avgCounter = 0;
	bool wasOnline = false;

	String getZeroFilled(int num);
};

