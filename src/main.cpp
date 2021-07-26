#include <WiFiClient.h>
#include <MQTTClient.h>
#include <ArduinoJson.h>
#include "WiFi.h"
#include <ETH.h>
#include "message-parser.cpp"
#include "artifactory-ota.h"

#define MQTT_FLUSH_INTERVAL_MS 100
#define MQTT_CONNECT_TIMEOUT 10000
#define MQTT_DEVICE_RESET_TIMEOUT 30000
#define START_RETRY_TIMEOUT_MS 3000
#define TAG_DISAPPEARED_TIMEOUT_MS 1500
#define SERIAL_MESSAGE_FAILED_TIMEOUT_MS 30000
#define OTA_CHECK_INTERVAL_MS 60000
#define NETWORK_CONNECTION_TIMEOUT_MS 15000

WiFiClient net = WiFiClient();
MQTTClient client = MQTTClient(4096);

String deviceId = "";
String hostname = "esp32-";

/**
 * Struct for tag registry items
 */
struct TagRegistryItem {
  std::string epc;
  unsigned long lastSeen;
  int16_t antenna;
};

bool ethConnected = false;
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

// Quere buffer
const uint16_t queueBufferSize = 100;
ContinueInventoryMessage queue[queueBufferSize];
uint16_t queueLength = 0;

uint16_t previousByte = 0;

// Device commands
const uint32_t stopAntennaCommand[8] = { 0xA5, 0x5A, 0x00, 0x08, 0x8C, 0x84, 0x0D, 0x0A };
const uint32_t startAntennaCommand[10] = { 0xA5, 0x5A, 0x00, 0x0A, 0x82, 0x27, 0x10, 0xBF, 0x0D, 0x0A };
const uint32_t askHWVersionCommand[8] = { 0xA5, 0x5A, 0x00, 0x08, 0x00, 0x08, 0x0D, 0x0A };
const uint32_t setRegionCommand[10] = { 0xA5, 0x5A, 0x00, 0x0A, 0x2C, 0x01, 0x04, 0x23, 0x0D, 0x0A };
const uint32_t setAntennasCommand[17] = { 0xa5, 0x5a, 0x00, 0x11, 0x28, 0x01, 0x00, 0x0f, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x37, 0x0d, 0x0a };
const uint32_t setIdleTimeCommand[11] = { 0xa5, 0x5a, 0x00, 0x0b, 0x4e, 0x01, 0x00, 0x32, 0x76, 0x0d, 0x0a };

unsigned long lastContinueAttempt = 0;
unsigned long lastMqttFlush = 0;
unsigned long lastOtaCheck = 0;
unsigned long lastMessageReceived = 0;
unsigned long lastMqttConnection = 0;

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
  String prefix = MQTT_TOPIC_PREFIX;
  String topic = MQTT_TOPIC;
  client.publish(prefix + "/" + topic + "/" + deviceId + "/" + antenna, jsonBuffer);
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
  for (int i = 0; i < 8; i++) {
    Serial1.write(stopAntennaCommand[i]);
  }
}

/**
 * Sends continue inventory command to device
 */
void continueInventory() {
  for (int i = 0; i < 10; i++) {
    Serial1.write(startAntennaCommand[i]);
  }
}

/**
 * Sends set region eu command to device
 */
void setEuRegion() {
  for (int i = 0; i < 10; i++) {
    Serial1.write(setRegionCommand[i]);
  }
}

/**
 * Sends set antennas command to device
 */
void setAntennas() {
  for (int i = 0; i < 17; i++) {
    Serial1.write(setAntennasCommand[i]);
  }
}

/**
 * Sends set idle time command to device
 */
void setIdleTime() {
  for (int i = 0; i < 11; i++) {
    Serial1.write(setIdleTimeCommand[i]);
  }
}

/**
 * Sends ask hardware version command to device
 */
void askHardwareVersion() {
  for (int i = 0; i < 8; i++) {
    Serial1.write(askHWVersionCommand[i]);
  }
}

/**
 * Connect to network
 */
void connectToNetwork() {
  if (ethConnected) {
    Serial.println("Ethernet connected, turning off Wi-Fi");
    WiFi.mode(WIFI_OFF);
  } else {
    WiFi.mode(WIFI_STA);
    WiFi.begin(WIFI_SSID, WIFI_PASS);
    Serial.print("Connecting to Wi-Fi (");
    Serial.print("ssid: ");
    Serial.print(WIFI_SSID);
    Serial.print(", pass: ");
    Serial.print(WIFI_PASS);
    Serial.println(")");
  }

  long connectionStarted = millis();
  while (WiFi.status() != WL_CONNECTED && !ethConnected && !net.connected()) {
    delay(500);
    Serial.print(".");
    if (millis() - connectionStarted > NETWORK_CONNECTION_TIMEOUT_MS) {
      Serial.println("Network connection timed out, retrying...");
      return;
    }
  }
  Serial.println("Network connected!");
}

/**
 * Handles ethernet events
 */
void onEthEvent(WiFiEvent_t event) {
  switch (event) {
    case SYSTEM_EVENT_ETH_START:
      Serial.print("ETH Started with hostname: ");
      Serial.println(hostname.c_str());
      ETH.setHostname(hostname.c_str());
      break;
    case SYSTEM_EVENT_ETH_CONNECTED:
      Serial.println("ETH Connected");
      break;
    case SYSTEM_EVENT_ETH_GOT_IP:
      Serial.print("ETH IPv4: ");
      Serial.print(ETH.localIP());
      if (ETH.fullDuplex()) {
        Serial.print(", FULL_DUPLEX");
      }
      Serial.print(", ");
      Serial.print(ETH.linkSpeed());
      Serial.println("Mbps");
      ethConnected = true;
      WiFi.disconnect();
      break;
    case SYSTEM_EVENT_ETH_DISCONNECTED:
      Serial.println("ETH Disconnected");
      ethConnected = false;
      break;
    case SYSTEM_EVENT_ETH_STOP:
      Serial.println("ETH Stopped");
      ethConnected = false;
      break;
    default:
      break;
  }
}

/**
 * Connect to MQTT 
*/
void connectToMQTT() {
  Serial.print("Setting MQTT settings (");
  Serial.print("user: ");
  Serial.print(MQTT_USER);
  Serial.print(", pass: ");
  Serial.print(MQTT_PASS);
  Serial.print(", topic prefix: ");
  Serial.print(MQTT_TOPIC_PREFIX);
  Serial.print(", topic: ");
  Serial.print(MQTT_TOPIC);
  Serial.print(", endpoint: ");
  Serial.print(MQTT_CHANNEL_ENDPOINT);
  Serial.println(")");

  client.begin(MQTT_CHANNEL_ENDPOINT, 1883, net);

  client.onMessage(messageHandler);
  client.setOptions(10, true, 5000);

  char clientId[deviceId.length() + 1];
  deviceId.toCharArray(clientId, deviceId.length() + 1);

  Serial.println("Connecting to MQTT endpoint...");
  long connectionStarted = millis();
  while (!client.connect(clientId, MQTT_USER, MQTT_PASS)) {
    Serial.println(client.lastError());
    Serial.print(".");
    delay(1000);
    if (!net.connected()) {
      connectToNetwork();
    }
    if (millis() - connectionStarted > MQTT_CONNECT_TIMEOUT) {
      return;
    }
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
  lastMessageReceived = millis();
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
 * Initializes serial communications
 */
void initializeCommunication() {
  stopInventory();
  delay(50);
  setEuRegion();
  delay(50);
  setAntennas();
  delay(50);
  setIdleTime();
  delay(50);
  continueInventory();
}

/**
 * Setup process
 */
void setup() {
  deviceId = WiFi.macAddress();
  hostname += deviceId;
  Serial.begin(9600);
  Serial1.begin(115200);
  Serial.println(deviceId);
  Serial.println(deviceId);
  WiFi.onEvent(onEthEvent);
  ETH.begin();
  connectToNetwork();
  connectToMQTT();
  delay(50);
  initializeCommunication();
}

/**
 * Main loop
 */
void loop() {
  if (millis() - lastMqttConnection > MQTT_DEVICE_RESET_TIMEOUT) {
    esp_restart();
  }

  if (!net.connected()) {
    Serial.println("Network connection lost, reconnecting...");
    connectToNetwork();
  }

  if (millis() - lastOtaCheck > OTA_CHECK_INTERVAL_MS) {
    lastOtaCheck = millis();
    checkFirmwareUpdates();
  }

  if (!client.connected()) {
    connectToMQTT();
  } else {
    lastMqttConnection = millis();
  }

  if (startSuccessfull && millis() - lastMessageReceived > SERIAL_MESSAGE_FAILED_TIMEOUT_MS) {
    startSuccessfull = false;
  }

  if (!startSuccessfull && millis() - lastContinueAttempt > START_RETRY_TIMEOUT_MS) {
    lastContinueAttempt = millis();
    initializeCommunication();
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
