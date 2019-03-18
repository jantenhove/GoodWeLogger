#pragma once
#include "SettingsManager.h"

class WifiConnector
{
  private:
    bool debugMode;
    bool tryingReconnect;
    bool hasWIFI;
    SettingsManager * settingsManager;

  public:
    WifiConnector(SettingsManager * settingsManager, bool debugMode = false);
    void start();
    void handle();
    void reconnect();
};