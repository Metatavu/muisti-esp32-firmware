#include <WiFiClient.h>
#include <MQTTClient.h>
#include <ArduinoJson.h>
#include "WiFi.h"
#include "secrets.h"
#include "message-parser.cpp"

#define MQTT_FLUSH_INTERVAL_MS 100
#define START_RETRY_TIMEOUT_MS 30000
#define TAG_DISAPPEARED_TIMEOUT_MS 1500

WiFiClient net = WiFiClient();
MQTTClient client = MQTTClient(4096);

String deviceId = "";

struct TagRegistryItem {
  std::string epc;
  unsigned long lastSeen;
  int16_t antenna;
};

bool wifiConnected = false;
static bool stopSuccessfull = false;
static bool startSuccessfull = false;

// Serial buffer
const uint32_t messageBufferSize = 256;
uint32_t antennaInputBuffer[messageBufferSize];
uint32_t antennaMessageLength = 0;

// Message markers
const uint16_t messageStartMarker[2] = { 0xA5, 0x5A };
const uint16_t messageEndMarker[2] = { 0x0D, 0x0A };

// Epc registry
const uint16_t registryBufferSize = 100;
TagRegistryItem registry[registryBufferSize];
uint16_t registryLength = 0;

//Quere buffer
const uint16_t queueBufferSize = 100;
ContinueInventoryMessage queue[queueBufferSize];
uint16_t queueLength = 0;

uint16_t previousByte = 0;

// Device commands
const uint32_t stopAntennaCommand[8] = { 0xA5, 0x5A, 0x00, 0x08, 0x8C, 0x84, 0x0D, 0x0A };
const uint32_t startAntennaCommand[10] = { 0xA5, 0x5A, 0x00, 0x0A, 0x82, 0x27, 0x10, 0xBF, 0x0D, 0x0A };
const uint32_t askHWVersionCommand[8] = { 0xA5, 0x5A, 0x00, 0x08, 0x00, 0x08, 0x0D, 0x0A };

unsigned long lastContinueAttempt = 0;
unsigned long lastMqttFlush = 0;

static MessageParser parser;

/**
 * Publishes message to mqtt broker
 * 
 * @param epc tag epc
 * @param strength signal strength
 * @param antenna antenna id
 */
void publishMQTTMessage(std::string epc, double strength, uint16_t antenna) {
  StaticJsonDocument<200> doc;
  doc["tag"] = epc;
  doc["strength"] = strength;
  char jsonBuffer[512];
  serializeJson(doc, jsonBuffer);
  client.publish(MQTT_TOPIC_PREFIX + "/" + MQTT_TOPIC + "/" + deviceId + "/" + antenna, jsonBuffer);
}

/**
 * Adds message received from serial to queue
 * also updates registry, if tag with same epc and antenna is found last seen value is updated to current time,
 * otherwise tag is added to registry
 * 
 * @param message messaged received from serial
 */
void addToQueue(ContinueInventoryMessage message) {
  boolean foundFromRegistry = false;
  for (uint16_t i = 0; i < registryLength; i++) {
    if (message.epc == registry[i].epc && message.antenna == registry[i].antenna) {
      registry[i].lastSeen = millis();
      foundFromRegistry = true;
    }
  }

  if (!foundFromRegistry) {
    registry[registryLength] = { epc: message.epc, lastSeen: millis(), antenna: message.antenna };
    registryLength++;
    if (registryLength >= registryBufferSize) {
      registryLength = registryBufferSize - 1;
      Serial.println("WARNING!! Epc registry overflow, losing data");
    }
  }

  for (uint16_t i = 0; i < queueLength; i++) {
    if (queue[i].antenna == message.antenna && queue[i].epc == message.epc) {
      queue[i] = message;
      return;
    }
  }

  if (queueLength >= queueBufferSize) {
    queueLength = queueBufferSize - 1;
    Serial.println("WARNING!! Message queue overflow, losing data");
  }
  queue[queueLength] = message;
  queueLength++;
}

/**
 * Flushes message queue to mqtt broker
 * Also checks if tags have not been seen 
 * in a while and sends message with strength of 0 for those tags
 */
void flushQueue() {
  for (uint16_t i = 0; i < queueLength; i++) {
    ContinueInventoryMessage message = queue[i];
    publishMQTTMessage(message.epc, message.strength, message.antenna);
  }
  queueLength = 0;

  unsigned long now = millis();
  uint16_t newRegistrySize = 0;
  for (uint16_t i = 0; i < registryLength; i++) {
    if (now - registry[i].lastSeen > TAG_DISAPPEARED_TIMEOUT_MS) {
      publishMQTTMessage(registry[i].epc, 0.0, registry[i].antenna);
    } else {
      registry[newRegistrySize] = registry[i];
      newRegistrySize++;
    }
  }
  registryLength = newRegistrySize;
}

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
  static uint16_t ndx = 0;
  static bool recvInProgress = false;
  uint16_t rc = Serial1.read();
  if (previousByte == messageStartMarker[0] && rc == messageStartMarker[1]) {
    // Message start detected
    recvInProgress = true;
    antennaInputBuffer[0] = previousByte;
    antennaInputBuffer[1] = rc;
    ndx = 2;
    antennaMessageLength = 0;
  } else if (recvInProgress && previousByte == messageEndMarker[0] && rc == messageEndMarker[1]) {
    // Message end detected
    if (ndx >= messageBufferSize - 3) {
      ndx = messageBufferSize - 3;
      Serial.println("WARNING!! input buffer overflow, losing data");
    }
    recvInProgress = false;
    antennaInputBuffer[ndx] = previousByte;
    antennaInputBuffer[ndx + 1] = rc;
    antennaMessageLength = ndx + 2;
    ndx = 0;
  } else if (recvInProgress) {
    if (ndx >= messageBufferSize) {
      ndx = messageBufferSize - 1;
      Serial.println("WARNING!! input buffer overflow, losing data");
    }
    antennaInputBuffer[ndx] = rc;
    ndx++;
  }
  previousByte = rc;
}

/**
 * Handles inventory response
 *
 * @param message parsed continue inventory response message
 */
void handleInventoryResponse(ContinueInventoryMessage message) {
  startSuccessfull = true;
  addToQueue(message);
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
  Serial.println(deviceId);

  connectToNetwork();
  connectToMQTT();
  //TODO: Add proper MQTT topic for OTA-update trigger
  // if (client.subscribe("/mqtt/muisti/beta")) {
  //   Serial.println("Succesfully subscribed to topic");
  // }
  stopInventory();
  continueInventory();
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

  if (!startSuccessfull && millis() - lastContinueAttempt > START_RETRY_TIMEOUT_MS) {
    lastContinueAttempt = millis();
    continueInventory();
  }

  if (Serial1.available()) {
    read();
  }

  if (antennaMessageLength > 0) {
    parseAntennaMessage();
    antennaMessageLength = 0;
  }

  if (millis() - lastMqttFlush > MQTT_FLUSH_INTERVAL_MS) {
    lastMqttFlush = millis();
    flushQueue();
  }

  client.loop();
}
