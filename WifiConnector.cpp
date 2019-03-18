#include "WifiConnector.h"
#include "Settings.h"
#include <ESP8266WiFi.h>
#include <ESP8266mDNS.h>
#include <ESP8266HTTPClient.h>
#include <WiFiUdp.h>
#include "ESP8266mDNS.h"

// Create Wifi Connector
WifiConnector::WifiConnector(SettingsManager * settingsMan, bool inDebug)
{
  debugMode = inDebug;
  settingsManager = settingsMan;
  tryingReconnect = false;
}

// Set params and try to connect
void WifiConnector::start()
{

  auto settings = settingsManager->GetSettings();

  if (debugMode){ 
    Serial.println("Start Wifi");
  }

  // Setup Wifi
  WiFi.mode(WIFI_STA);
	WiFi.hostname(settings->wifiHostname.c_str());
	WiFi.begin(settings->wifiSSID.c_str(), settings->wifiPassword.c_str());

  // This funtion is called from main setup function,
  // Wait until unit has wifi before continuing
  while (WiFi.status() != WL_CONNECTED) {
      delay(500);
      if (debugMode){
        Serial.print(".");
      }
  }

  // Set wifi bool
  hasWIFI = true;

  if (debugMode)
  {
    Serial.println("Wifi Connected!");
    Serial.println("IP: ");
    Serial.println(WiFi.localIP());
  }

}

// If wifi connection is lost, try to reconnect
void WifiConnector::reconnect()
{

  if (debugMode){
    Serial.println("Try to reconnect");
  }
  
  // First hit, try to reconnect. 
  if(!tryingReconnect)
  {
    WiFi.reconnect();
    tryingReconnect = true;
  }

}

// handle function called from main loop
void WifiConnector::handle()
{
    
  // Check if WIfi is Connected
  if(!WiFi.isConnected() && !tryingReconnect){
        
    if (debugMode){
      Serial.println("Wifi disconnected");
    }

    // Set bool false and try to reconnect
    hasWIFI = false;
    reconnect();
 
  }else if (WiFi.isConnected() && !hasWIFI){ // Wifi is Reconnected

    if (debugMode){
      Serial.println("Wifi reconnected");
    }

    hasWIFI = true;
    tryingReconnect = false;

  }
        
}