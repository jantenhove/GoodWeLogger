#include <TimeLib.h>
#include <NTPClient.h>
#include <ESP8266WiFi.h>
#include <ESP8266mDNS.h>
#include <ESP8266HTTPClient.h>
#include <WiFiUdp.h>
#include <ArduinoOTA.h>
#include "GoodWeCommunicator.h"
#include "SettingsManager.h"
#include "MQTTPublisher.h"
#include "PVOutputPublisher.h"
#include "Settings.h"			//change and then rename Settings.example.h to Settings.h to compile
#include "Debug.h"

SettingsManager settingsManager;
GoodWeCommunicator goodweComms(&settingsManager);
MQTTPublisher mqqtPublisher(&settingsManager, &goodweComms);
PVOutputPublisher pvoutputPublisher(&settingsManager, &goodweComms);
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, NTP_SERVER);
bool validTimeSet = false;
int reconnectCounter = 0;
#define WDT_TIMEOUT 600  // 600 seconds = 10 minutes


#ifdef REMOTE_DEBUGGING_ENABLED
RemoteDebug Debug;
#endif

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
	settings->wifiHostname = WIFI_HOSTNAME;
	settings->wifiSSID = WIFI_SSID;
	settings->wifiPassword = WIFI_PASSWORD;
	settings->timezone = TIMEZONE;
	settings->RS485Rx = RS485_RX;
	settings->RS485Tx = RS485_TX;
	settings->wifiConnectTimeout = WIFI_CONNECT_TIMEOUT;
	settings->ntpServer = NTP_SERVER;
	settings->inverterOfflineDataResetTimeout = INVERTER_OFFLINE_RESET_VALUES_TIMEOUT;

	//Init our compononents
	Serial.begin(115200);
	Serial.println("Booting");
	WiFi.mode(WIFI_STA);
	WiFi.hostname(settings->wifiHostname.c_str());
	WiFi.begin(settings->wifiSSID.c_str(), settings->wifiPassword.c_str());

	//check wifi connection
	if (!checkConnectToWifi())
	{
		Serial.println("Cannot establish WiFi connection Please check the Wifi settings in the header file. Restarting ESP");
		ESP.restart();
	}

	timeClient.setUpdateInterval(1000 * 60 * 60); //one hour updates
	timeClient.begin();

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

#ifdef REMOTE_DEBUGGING_ENABLED
	Debug.begin("GoodweLogger");
	Debug.setResetCmdEnabled(true);
	Debug.setCallBackNewClient(&RemoteDebugClientConnected);
#endif

	//ntp client
	goodweComms.start();
	mqqtPublisher.start();
	validTimeSet = timeClient.update();
	timeClient.setTimeOffset(settings->timezone * 60 * 60);
    // Initialize the watchdog timer
    esp_task_wdt_init(WDT_TIMEOUT, true); // Enable panic so ESP32 restarts
    esp_task_wdt_add(NULL); // Add the current task to the watchdog
}


#ifdef REMOTE_DEBUGGING_ENABLED
void RemoteDebugClientConnected()
{
	//Debug client connected. Print all our info
	debugPrintln("===GoodWeLogger remote debug enabled===");
	
}
#endif

bool checkConnectToWifi()
{
	//check for wifi connection
	if (WiFi.status() != WL_CONNECTED || WiFi.localIP() == IPAddress(0, 0, 0, 0))
	{
		Serial.println("Connecting to WIFI...");
		int totalWaitTime = 0;
		while (WiFi.status() != WL_CONNECTED)
		{ // Wait for the Wi-Fi to connect
			delay(1000);
			totalWaitTime += 1000;
			Serial.print(".");
			if (totalWaitTime > WIFI_CONNECT_TIMEOUT)
			{
				//no connection.
				Serial.println("\nWiFi connect timed out");
				return false;
			}
		}
	}
	return true;
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

	//check wifi connection
	if (!checkConnectToWifi())
	{
		Serial.println("Wifi connection lost for too long. Restarting ESP");
		ESP.restart();
	}

	ArduinoOTA.handle();
	yield();
	goodweComms.handle();
	yield();
	mqqtPublisher.handle();
	yield();
	//start the pvoutput publisher after the time has been set if it is configured to start
	if (validTimeSet && pvoutputPublisher.canStart() && !pvoutputPublisher.getIsStarted())
		pvoutputPublisher.start();

	pvoutputPublisher.handle();
	yield(); //prevent wathcdog timeout
	
	// Feed the watchdog
    esp_task_wdt_reset();

#ifdef REMOTE_DEBUGGING_ENABLED
	Debug.handle();
#endif
}
