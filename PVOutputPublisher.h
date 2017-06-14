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

	String getZeroFilled(int num);
};

