#ifndef ARTIFACTORY_OTA_H
#define ARTIFACTORY_OTA_H

#include <Arduino.h>

void checkFirmwareUpdates();
void processOTAUpdate(const String &version);

#endif // SECURE_OTA_H