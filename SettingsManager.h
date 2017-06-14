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


		int timezone;

		String pvoutputApiKey;
		String pvoutputSystemId;
		int pvoutputUpdateInterval;

		String wifiSSID;
		String wifiPassword;

	};
	SettingsManager();
	~SettingsManager();

	Settings * GetSettings();
};

