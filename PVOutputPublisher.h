#pragma once
#include <Arduino.h>
#include "TimeLib.h"
#include <ESP8266WiFi.h>
#include "SettingsManager.h"
#include "GoodWeCommunicator.h"
#include "ESP8266HTTPClient.h"
#include "Debug.h"

#define MAX_EDAY_DIFF 100.0f

class PVOutputPublisher
{
public:
	PVOutputPublisher(SettingsManager * settingsManager, GoodWeCommunicator *goodWe);
	~PVOutputPublisher();

	void start();
	void stop();
	bool canStart();
	bool getIsStarted();
	void sendToPvOutput(GoodWeCommunicator::GoodweInverterInformation info);

	void handle();

	void ResetAverage();

private:
	SettingsManager::Settings * pvoutputSettings;
	SettingsManager * pvOutputSettingsManager;
	GoodWeCommunicator * goodweCommunicator;
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
	float prevEday = 0.0f;
	String getZeroFilled(int num);
};

