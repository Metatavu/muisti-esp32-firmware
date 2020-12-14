#include <WiFiClient.h>
#include <MQTTClient.h>
#include <ArduinoJson.h>
#include "WiFi.h"
#include "FS.h"
#include "SPIFFS.h"
#include "SD_MMC.h"
#include "secrets.h"
#include "message-parser.cpp"

WiFiClient net = WiFiClient();
MQTTClient client = MQTTClient(4096);

String deviceId = "";

bool wifiConnected = false;
static bool recvInProgress = false;
static bool stopSuccessfull = false;
static bool startSuccessfull = false;

const uint32_t messageBufferSize = 256;
uint32_t antennaInputBuffer[messageBufferSize];
uint32_t antennaMessageLength = 0;

// Message markers
const uint32_t messageStartMarker = 0xA5;
const uint32_t messageEndMarker[2] = { 0x0D, 0x0A };

// Device commands
const uint32_t stopAntennaCommand[8] = { 0xA5, 0x5A, 0x00, 0x08, 0x8C, 0x84, 0x0D, 0x0A };
const uint32_t startAntennaCommand[10] = { 0xA5, 0x5A, 0x00, 0x0A, 0x82, 0x27, 0x10, 0xBF, 0x0D, 0x0A };
const uint32_t askHWVersionCommand[8] = { 0xA5, 0x5A, 0x00, 0x08, 0x00, 0x08, 0x0D, 0x0A };

static MessageParser parser({});

/**
 * MQTT message handler
 */
void messageHandler(String &topic, String &payload) {
  Serial.println("incoming: " + topic + " - " + payload);
}

/**
 * Sends stop inventory command to device
 */
void stopInventory() {
  Serial.println("Sending stop continue inventory command...");
  for (int i = 0; i < 8; i++) {
    Serial1.write(stopAntennaCommand[i]);
  }
  Serial.println("Sent stop inventory command");
}

/**
 * Sends continue inventory command to device
 */
void continueInventory() {
  Serial.println("Sending continue inventory command...");
  for (int i = 0; i < 10; i++) {
    Serial1.write(startAntennaCommand[i]);
  }
  Serial.println("Sent continue inventory command");
}

/**
 * Sends ask hardware version command to device
 */
void askHardwareVersion() {
  Serial.println("Sending ask hardware version command...");
  for (int i = 0; i < 8; i++) {
    Serial1.write(askHWVersionCommand[i]);
  }
  Serial.println("Sent ask hardware versioncommand");
}

/**
 * Connect to network
 */
void connectToNetwork() {
  WiFi.mode(WIFI_STA);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  Serial.println("Connecting to Wi-Fi");

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("Wifi connected!");
}

/**
 * Connect to MQTT 
*/
void connectToMQTT() {
  Serial.println("Setting MQTT settings");

  client.begin(MQTT_CHANNEL_ENDPOINT, 1883, net);

  client.onMessage(messageHandler);
  client.setOptions(10, true, 5000);

  char clientId[deviceId.length() + 1];
  deviceId.toCharArray(clientId, deviceId.length() + 1);

  Serial.println("Connecting to MQTT endpoint...");
  while (!client.connect(clientId, MQTT_USER, MQTT_PASS)) {
    Serial.println(client.lastError());
    Serial.print(".");
    delay(1000);
  }

  Serial.println("MQTT connected!");
}

/**
 * Read message from antenna
 */
void read() {
  static uint32_t ndx = 0;
  static bool firstEndMarkerFound = false;
  static bool secondEndMarkerFound = false;
  uint32_t rc = Serial1.read();
  if (recvInProgress == true) {

    if (rc == messageEndMarker[0]) {
      firstEndMarkerFound = true;
      antennaInputBuffer[ndx] = rc;
    } else if (rc == messageEndMarker[1]) {
      secondEndMarkerFound = true;
      antennaInputBuffer[ndx] = rc;
    } else {
      firstEndMarkerFound = false;
      secondEndMarkerFound = false;
      antennaInputBuffer[ndx] = rc;
      if (ndx >= messageBufferSize) {
        Serial.println(ndx);
        ndx = messageBufferSize - 1;
      }
    }

    if (firstEndMarkerFound && secondEndMarkerFound) {
      recvInProgress = false;
      antennaMessageLength = ndx + 1;
      ndx = 0;
    }

  } else if (rc == messageStartMarker) {
    recvInProgress = true;
    firstEndMarkerFound = false;
    secondEndMarkerFound = false;
    ndx = 0;
    antennaMessageLength = 0;
    antennaInputBuffer[ndx] = rc;
  } else {
    Serial.println("Got incorrect start hex!");
  }
  ndx++;
}

/**
 * Handles inventory response
 *
 * @param message parsed continue inventory response message
 */
void handleInventoryResponse(ContinueInventoryMessage message) {
  startSuccessfull = true;
  StaticJsonDocument<200> doc;
  doc["tag"] = message.getEpc();
  doc["strength"] = message.getStrength();
  char jsonBuffer[512];
  serializeJson(doc, jsonBuffer);
  client.publish(MQTT_TOPIC_PREFIX + "/" + MQTT_TOPIC + "/" + deviceId + "/" + message.getAntenna(), jsonBuffer);
}

/**
 * Parse message with given type
 * TODO: Add support for other message types
 *
 * @param type type hex 
 * @param message antenna message
 * @param parser initialized parser
 */
void parseMessageWithType(uint32_t type, uint32_t message[], MessageParser parser) {
  switch (type) {
  case CONTINUE_INVENTORY_RESPONSE:
    handleInventoryResponse(parser.parseContinueInventoryResponse(message));
    break;
  case STOP_CONTINUE_INVENTORY_RESPONSE:
    stopSuccessfull = parser.parseStopContinueInventoryResponse(message);
    break;
  default:
    break;
  }
}

/**
 * Prints device hex message to console. Leave this here for debugging
 */
void printDeviceHexMessage() {
  Serial.println("----------START OF DEVICE MESSAGE----------");
  for (uint32_t i = 0; i < antennaMessageLength; i++) {
    Serial.println(antennaInputBuffer[i]);
  }
  Serial.println("----------END OF DEVICE MESSAGE----------");
}

/**
 * Parse antenna message
 */
void parseAntennaMessage() {
  if (!parser.checkMessageStart(antennaInputBuffer)) {
    Serial.println("Message start header was incorrect!!\n");
    return;
  }

  if (!parser.checkMessageEnd(antennaInputBuffer, antennaMessageLength)) {
    Serial.println("Message end was incorrect!!\n");
    return;
  }

  if (!parser.checkCRC(antennaInputBuffer)) {
    Serial.println("Message CRC was incorrect!!!\n");
    return;
  }

  parseMessageWithType(antennaInputBuffer[4], antennaInputBuffer, parser);
}

/**
 * Setup process
 */
void setup() {
  deviceId = WiFi.macAddress();
  Serial.begin(9600);
  Serial1.begin(115200);

  connectToNetwork();
  connectToMQTT();
  //TODO: Add proper MQTT topic for OTA-update trigger
  // if (client.subscribe("/mqtt/muisti/beta")) {
  //   Serial.println("Succesfully subscribed to topic");
  // }
  stopInventory();
}

/**
 * Main loop
 */
void loop() {
  
  if (!net.connected()) {
    connectToNetwork();
  }

  if (!client.connected()) {
    connectToMQTT();
  }

  if (!startSuccessfull) {
    continueInventory();
  }

  while (Serial1.available()) {
    read();
  }

  if (antennaMessageLength > 0 && !recvInProgress) {
    parseAntennaMessage();
    antennaMessageLength = 0;
  }

  client.loop();
}
