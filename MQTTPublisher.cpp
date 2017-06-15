#include "MQTTPublisher.h"

WiFiClient espClient;
PubSubClient client(espClient);

MQTTPublisher::MQTTPublisher(SettingsManager * settingsManager, GoodWeCommunicator * goodWe, bool inDebugMode)
{
	randomSeed(micros());
	mqttSettingsManager = settingsManager;
	goodweCommunicator = goodWe;
	debugMode = inDebugMode;
}

MQTTPublisher::~MQTTPublisher()
{
	client.publish("goodwe", "offline");
	client.disconnect();
}


bool MQTTPublisher::reconnect()
{
	lastConnectionAttempt = millis();
	if (debugMode)
	{
		Serial.print("Attempting MQTT connection to server: ");
		Serial.print(mqttSettings->mqttHostName);
		Serial.print("...");
	}

	// Create a random client ID
	String clientId = "GoodWeLogger-";
	clientId += String(random(0xffff), HEX);

	// Attempt to connect
	bool clientConnected;
	if (mqttSettings->mqttUserName.length())
	{
		Serial.println("Using user credientials for authentication.");
		clientConnected = client.connect(clientId.c_str(), mqttSettings->mqttUserName.c_str(), mqttSettings->mqttPassword.c_str());
	}
	else
	{
		Serial.println("Connecting without user credentials.");
		clientConnected = client.connect(clientId.c_str());
	}
		
	if (clientConnected)
	{
		if (debugMode)
			Serial.println("connected");
		// Once connected, publish an announcement...
		client.publish("goodwe", "online");

		return true;
	}
	else {
		if (debugMode)
		{
			Serial.print("failed, rc=");
			Serial.print(client.state());
		}
	}

	return false;
}


void MQTTPublisher::start()
{
	mqttSettings = mqttSettingsManager->GetSettings();
	if (mqttSettings->mqttHostName.length() == 0 || mqttSettings->mqttPort == 0)
	{
		Serial.println("MQTT disabled. No hostname or port set.");
		return; //not configured
	}
	Serial.println("MQTT enabled. Connecting.");
	client.setServer(mqttSettings->mqttHostName.c_str(), mqttSettings->mqttPort);
	reconnect(); //connect right away
	isStarted = true;
}

void MQTTPublisher::stop()
{
	isStarted = false;
}

void MQTTPublisher::handle()
{
	if (!isStarted)
		return;

	if (!client.connected() && millis() - lastConnectionAttempt > RECONNECT_TIMEOUT) {
		if (!reconnect()) return;
	}
	//got a valid mqtt connection. Loop through the inverts and send out the data if needed
	client.loop();
	bool sendRegular = millis() - lastSentRegularUpdate > mqttSettings->mqttRegularUpdateInterval;
	bool sendQuick = millis() - lastSentQuickUpdate > mqttSettings->mqttQuickUpdateInterval;

	if (sendRegular || sendQuick)
	{
		auto inverters = goodweCommunicator->getInvertersInfo();
		for (char cnt = 0; cnt < inverters.size(); cnt++)
		{
			if (inverters[0].addressConfirmed)
			{
				auto prependTopic = (String("goodwe/") + String(inverters[cnt].serialNumber));
				if (debugMode)
				{
					Serial.print("Publishing prepend topic for this inverter is: ");
					Serial.println(prependTopic);
				}

				if (inverters[cnt].isOnline)
				{
					if (sendQuick)
					{
						//send out fast changing values
						client.publish((prependTopic.c_str() + String("/online")).c_str(), String(1).c_str());
						client.publish((prependTopic.c_str() + String("/vpv1")).c_str(), String(inverters[cnt].vpv1, 1).c_str());
						client.publish((prependTopic.c_str() + String("/vpv2")).c_str(), String(inverters[cnt].vpv2, 1).c_str());
						client.publish((prependTopic.c_str() + String("/ipv1")).c_str(), String(inverters[cnt].ipv1, 1).c_str());
						client.publish((prependTopic.c_str() + String("/ipv2")).c_str(), String(inverters[cnt].ipv2, 1).c_str());
						client.publish((prependTopic.c_str() + String("/vac1")).c_str(), String(inverters[cnt].vac1, 1).c_str());
						client.publish((prependTopic.c_str() + String("/iac1")).c_str(), String(inverters[cnt].iac1, 1).c_str());
						client.publish((prependTopic.c_str() + String("/fac1")).c_str(), String(inverters[cnt].fac1, 2).c_str());
						client.publish((prependTopic.c_str() + String("/pac")).c_str(), String(inverters[cnt].pac).c_str());
						client.publish((prependTopic.c_str() + String("/temp")).c_str(), String(inverters[cnt].temp, 1).c_str());
						client.publish((prependTopic.c_str() + String("/eday")).c_str(), String(inverters[cnt].eDay).c_str());
						if (inverters[cnt].isDTSeries)
						{
							//also send tri fase info
							client.publish((prependTopic.c_str() + String("/vac2")).c_str(), String(inverters[cnt].vac2, 1).c_str());
							client.publish((prependTopic.c_str() + String("/iac2")).c_str(), String(inverters[cnt].iac2, 1).c_str());
							client.publish((prependTopic.c_str() + String("/fac2")).c_str(), String(inverters[cnt].iac2, 2).c_str());
							client.publish((prependTopic.c_str() + String("/vac3")).c_str(), String(inverters[cnt].vac3, 1).c_str());
							client.publish((prependTopic.c_str() + String("/iac3")).c_str(), String(inverters[cnt].iac3, 1).c_str());
							client.publish((prependTopic.c_str() + String("/fac3")).c_str(), String(inverters[cnt].iac3, 2).c_str());
						}
					}
					else
					{
						//regular
						client.publish((prependTopic.c_str() + String("/workmode")).c_str(), String(inverters[cnt].workMode).c_str());
						//TODO: Rest of data 
					}
				}
				else if(sendRegular) //only send offline info on regular basis
					client.publish((prependTopic.c_str() + String("/online")).c_str(), String(0).c_str());
			}

		}

		if (sendQuick)
			lastSentQuickUpdate = millis();
		if (sendRegular)
			lastSentRegularUpdate = millis();
	}



}
