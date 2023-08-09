#include <HTTPClient.h>
#include <Update.h>
#include "ota-update.h"

String updatesUrl = UPDATES_URL;
const char *versionName = VERSION_NAME;

// Variables to validate firmware content
volatile int contentLength = 0;
volatile bool isValidContentType = false;

/**
 * Parses version string to integer. Method expects version string in format x.y.z. 
 * 
 * If version string contains more than 3 parts, only first 3 parts are used. So for example version with branch name still returns the same version as version without branch name (e.g. 1.2.19-feature-14-new-distribution-system.0 will return 102019)
 */
int parseVersion(const char *versionString) {
    int firstPart = 0, secondPart = 0, thirdPart = 0;
    sscanf(versionString, "%d.%d.%d", &firstPart, &secondPart, &thirdPart);
    return firstPart * 100000 + secondPart * 1000 + thirdPart;
}

/**
 * Returns current firmware version
 */
int getCurrentVersion() {
  return parseVersion(versionName);
}

/**
 * Returns url used to query latests firmware version
 * @return url to latest version query
 */
String getVersionUrl() {
  return updatesUrl + "/version.txt";
}

/**
 * Performs query to check the latest firmware version and returns the latest version
 * @return latest version as string
 */
String getLatestVersion() {
  HTTPClient http;
  http.begin(getVersionUrl());
  int httpResponseCode = http.GET();
  if (httpResponseCode != 200) {
    Serial.println("Failed to load firmware version");
    Serial.println(httpResponseCode);
    http.end();
    return "";
  }
  String version = http.getString();
  http.end();
  return version;
}

/**
 * Gets path to firmware file by version
 * @param version Firmware version
 * @return path to firmware
 */
String getFirmwarePath(String version) {
  return updatesUrl + "/" + version + "/firmware.bin";
}

/**
 * Gets file location from firmware location header
 * @param version firmware version
 * @return path to firmware file
 */
String getFileUrl(String version) {
  String firmwarePath = getFirmwarePath(version);
  HTTPClient http;
  const char * headerKeys[] = {"Location"};
  http.begin(firmwarePath);
  http.collectHeaders(headerKeys, 1);
  int httpResponseCode = http.GET();
  if (httpResponseCode <= 0) {
    Serial.println("Failed to get file url");
    return "";
  }
  String location = http.header("Location");
  http.end();
  return location;
}

/**
 * Checks and updates firmware if new version is available
 */
void checkFirmwareUpdates() {
  // Fetch the latest firmware version
  String latestVersionName = getLatestVersion();
  int latestVersion = parseVersion(latestVersionName.c_str());
  if (latestVersion <= getCurrentVersion()) {
    Serial.println("Current firmware is up to date");
    return;
  }

  Serial.println("There is a new version of firmware available: " + latestVersionName);
  processOTAUpdate(latestVersionName);
}

/**
 * OTA update processing
 * 
 * @param version firmware version
 */
void processOTAUpdate(const String &version) {
  String fileUrl = getFileUrl(version);
  if (fileUrl.length() == 0) {
    Serial.println("Failed to resolve file url");
    return;
  }

  HTTPClient http;
  const char * headerKeys[] = {"Content-Type"};
  http.begin(fileUrl);
  http.collectHeaders(headerKeys, 1);
  int httpResponseCode = http.GET();
  if (httpResponseCode > 0) {
    contentLength = http.getSize();
    String contentType = http.header("Content-Type");
    if (contentType == "application/octet-stream") {
      isValidContentType = true;
    }
  }

  // check whether we have everything for OTA update
  if (contentLength && isValidContentType) {
    if (Update.begin(contentLength)) {
      Serial.println("Starting Over-The-Air update. This may take some time to complete ...");
      size_t written = Update.writeStream(http.getStream());

      if (written == contentLength) {
        Serial.println("Written : " + String(written) + " successfully");
      } else {
        Serial.println("Written only : " + String(written) + "/" + String(contentLength) + ". Retry?");
        // Retry??
      }

      if (Update.end()) {
        if (Update.isFinished()) {
          Serial.println("OTA update has successfully completed. Rebooting ...");
          ESP.restart();
        } else {
          Serial.println("Something went wrong! OTA update hasn't been finished properly.");
        }
      } else {
        Serial.println("An error Occurred. Error #: " + String(Update.getError()));
      }
    } else {
      Serial.println("There isn't enough space to start OTA update");
      http.end();
    }
  } else {
    Serial.println("There was no valid content in the response from the OTA server!");
    http.end();
  }
}
