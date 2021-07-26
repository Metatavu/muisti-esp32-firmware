#include <iostream>
#include <array>
#include <sstream>
#include <cstdio>
#include <string.h>
#include <iomanip>
#include <sstream>
#include "../src/message-parser.cpp"

uint32_t antennaStoppedMessageLength = 9;
uint32_t antennaStoppedMessage[9] = { 0xA5, 0x5A, 0x00, 0x09, 0x8D, 0x01, 0x85, 0x0D, 0x0A };

uint32_t continueInventoryMessageLength = 29;
uint32_t continueInventoryMessage[29] = {
  // Message start
  0xA5, 0x5A,
  // Message length
  0x00, 0x1D,
  // Command
  0x83,
  // PC
  0x30, 0x00,
  // EPC
  0xE2, 0x00, 0x34, 0x11, 0xB8, 0x02, 0x01, 0x13, 0x83, 0x25, 0x85, 0x66,
  // RSSI
  0xFD, 0x6F,
  // Antenna number
  0x02,
  // Frequency
  0x0D, 0xF7, 0x32,
  // Phase
  0x2D,
  // Check code
  0xF1,
  // Message end
  0x0D, 0x0A
};

void handleResponse(ContinueInventoryMessage message) {
  message.printValues();
}

/**
 * Parse message with given type
 * TODO: Add support for other message types and possibly move message type specific
 * parsing to individual files
 *
 * @param type type hex 
 * @param message antenna message
 */
void parseMessageWithType(uint32_t type, uint32_t message[], MessageParser parser) {
  switch (type) {
  case CONTINUE_INVENTORY_RESPONSE:
    std::cout << "Message type was continue inventory response\n";
    handleResponse(parser.parseContinueInventoryResponse(message));
    break;
  case STOP_CONTINUE_INVENTORY_RESPONSE:
    std::cout << "Message type was stop continue inventory response\n";
    parser.parseStopContinueInventoryResponse(message);
    break;
  default:
    break;
  }
}

/**
 * Parse antenna message
 */
void parseMessage(uint32_t message[], uint32_t messageLength) {
  MessageParser parser({});
  if (!parser.checkMessageStart(message)) {
    std::cout << "Message start header was incorrect!!\n";
    return;
  }

  if (!parser.checkMessageEnd(message, messageLength)) {
    std::cout << "Message end was incorrect!!\n";
    return;
  }

  if (!parser.checkCRC(message)) {
    std::cout << "Message CRC was incorrect!!!\n";
    return;
  }

  parseMessageWithType(message[4], message, parser);
}

/**
 * Test message parsing logic by running command:
 * g++ test/test.cpp && ./a.out
 * from project root
 */
int main() {
  parseMessage(continueInventoryMessage, continueInventoryMessageLength);
  parseMessage(antennaStoppedMessage, antennaStoppedMessageLength);
  return 0;
}