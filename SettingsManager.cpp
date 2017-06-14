#include "SettingsManager.h"


SettingsManager::Settings applicationSettings;

SettingsManager::SettingsManager()
{
}


SettingsManager::~SettingsManager()
{
}

SettingsManager::Settings * SettingsManager::GetSettings()
{
	return &applicationSettings;
}
