#include <HTTPClient.h>
#include <Update.h>
#include "artifactory-ota.h"

String organization = ARTIFACTORY_ORGANIZATION;
String module = ARTIFACTORY_MODULE;
String repository = ARTIFACTORY_REPOSITORY;
String token = ARTIFACTORY_API_TOKEN;

// Variables to validate firmware content
volatile int contentLength = 0;
volatile bool isValidContentType = false;

/**
 * Returns url used to query latests firmware version
 * @return url to latest version query
 */
String getVersionUrl() {
  return "https://metatavu.jfrog.io/artifactory/api/search/latestVersion?g=" + organization + "&a=" + module + "&repos=" + repository;
}

/**
 * Performs query to check the latest firmware version and returns the latest version
 * @return latest version as string
 */
String getLatestVersion() {
  HTTPClient http;
  http.begin(getVersionUrl());
  http.addHeader("Authorization", "Bearer " + token);
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
  return "https://metatavu.jfrog.io/artifactory/" + repository + "/" + organization + "/"+ module + "/" + module + "-" + version + ".bin";
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
  http.addHeader("Authorization", "Bearer " + token);
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
  const String latest = getLatestVersion();
  if (latest.length() == 0) {
    Serial.println("Could not load info about the latest firmware, so nothing to update. Continue ...");
    return;
  } else if (atoi(latest.c_str()) <= VERSION) {
    return;
  }

  Serial.println("There is a new version of firmware available: v." + latest);
  processOTAUpdate(latest);
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
