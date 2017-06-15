#include <TimeLib.h>
#include <NTPClient.h>
#include <PubSubClient.h>
#include <ESP8266WiFi.h>
#include <ESP8266mDNS.h>
#include <ESP8266HTTPClient.h>
#include <WiFiUdp.h>
#include <ArduinoOTA.h>
#include "GoodWeCommunicator.h"
#include "SettingsManager.h"
#include "MQTTPublisher.h"
#include "PVOutputPublisher.h"
#include "ESP8266mDNS.h"
#include "Settings.h"			//change and then rename Settings.example.h to Settings.h to compile


GoodWeCommunicator goodweComms(D1, D2, true);
SettingsManager settingsManager;
MQTTPublisher mqqtPublisher(&settingsManager, &goodweComms, true);
PVOutputPublisher pvoutputPublisher(&settingsManager, &goodweComms, true);
WiFiUDP ntpUDP;

NTPClient timeClient(ntpUDP, "pool.ntp.org");
bool validTimeSet =false;

void setup()
{
	//debug settings
	auto settings = settingsManager.GetSettings();
	//set settings from heade file
	settings->mqttHostName = MQTT_HOST_NAME;	
	settings->mqttPort = MQTT_PORT;	
	settings->mqttUserName = MQTT_USER_NAME;
	settings->mqttPassword = MQTT_PASSWORD;	
	settings->mqttQuickUpdateInterval = MQTT_QUICK_UPDATE_INTERVAL;	
	settings->mqttRegularUpdateInterval = MQTT_REGULAR_UPDATE_INTERVAL;	
	settings->pvoutputApiKey = PVOUTPUT_API_KEY;	
	settings->pvoutputSystemId = PVOUTPUT_SYSTEM_ID;	
	settings->pvoutputUpdateInterval = PVOUTPUT_UPDATE_INTERVAL;	
	settings->wifiSSID = WIFI_SSID;
	settings->wifiPassword = WIFI_PASSWORD;
	settings->timezone = TIMEZONE;

	/* add setup code here */
	Serial.begin(115200);
	Serial.println("Booting");
	WiFi.mode(WIFI_STA);
	WiFi.begin(settings->wifiSSID.c_str(), settings->wifiPassword.c_str());

	Serial.print("Connecting to WiFi");
	while (WiFi.status() != WL_CONNECTED) {
		delay(500);
		Serial.print(".");
	}
	Serial.println("");
	Serial.println("Connected!");

	timeClient.begin();


	// Hostname defaults to esp8266-[ChipID]
	ArduinoOTA.setHostname("GoodWeLogger");

	ArduinoOTA.onStart([]() {
		Serial.println("Start Ota");
	});
	ArduinoOTA.onEnd([]() {
		Serial.println("\nEnd Ota");
	});
	ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {
		Serial.printf("OTA Progress: %u%%\r", (progress / (total / 100)));
	});
	ArduinoOTA.onError([](ota_error_t error) {
		Serial.printf("OTA Error[%u]: ", error);
		if (error == OTA_AUTH_ERROR) Serial.println("Auth Failed");
		else if (error == OTA_BEGIN_ERROR) Serial.println("Begin Failed");
		else if (error == OTA_CONNECT_ERROR) Serial.println("Connect Failed");
		else if (error == OTA_RECEIVE_ERROR) Serial.println("Receive Failed");
		else if (error == OTA_END_ERROR) Serial.println("End Failed");
	});
	ArduinoOTA.begin();
	Serial.println("Ready");
	Serial.println("IP address: ");
	Serial.println(WiFi.localIP());


	

	//ntp client
	goodweComms.start();
	mqqtPublisher.start();
	validTimeSet = timeClient.update();
	timeClient.setTimeOffset(settings->timezone * 60 * 60);

}


void loop()
{
	//update the time if set correctly
	auto timeUpdate = timeClient.update();

	//if the time is updated, we need to set it in the time lib so we can use the helper functions from timelib
	if (timeUpdate)
	{
		if (!validTimeSet)
			validTimeSet = true; //pvoutput is started after the time is set
	
		//sync time to time lib
		setTime(timeClient.getEpochTime());
	}

	ArduinoOTA.handle();
	goodweComms.handle();
	mqqtPublisher.handle();
	//start the pvoutput publisher after the time has been set if it is configured to start
	if (validTimeSet && pvoutputPublisher.canStart() && !pvoutputPublisher.getIsStarted())
	{
		pvoutputPublisher.start();
	}

	pvoutputPublisher.handle();
	delay(0);
}
