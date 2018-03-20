#pragma once
#include <ESP8266WiFi.h>

class SettingsManager
{
public:
	struct Settings
	{
	public:
		//mqtt settings
		String mqttHostName;
		int mqttPort;
		String mqttUserName;
		String mqttPassword;
		int mqttQuickUpdateInterval;
		int mqttRegularUpdateInterval;

		//pvoutput settings
		String pvoutputApiKey;
		String pvoutputSystemId;
		int pvoutputUpdateInterval;

		//wifi settings
		String wifiHostname;		//Set hostname for ESP8266
		String wifiSSID;
		String wifiPassword;

		//general settings
		int RS485Rx =D1;		//default set because added later
		int RS485Tx =D2;
		int timezone;

	};
	SettingsManager();
	~SettingsManager();

	Settings * GetSettings();
};

