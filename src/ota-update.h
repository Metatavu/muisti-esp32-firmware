#ifndef OTA_UPDATE_H
#define OTA_UPDATE_H

#include <Arduino.h>

void checkFirmwareUpdates();
void processOTAUpdate(const String &version);

#endif // OTA_UPDATE_H