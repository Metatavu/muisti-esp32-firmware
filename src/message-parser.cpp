#include <iostream>
#include <array>
#include <sstream>
#include <cstdio>
#include <string.h>
#include <iomanip>
#include <sstream>
#include "./message-types.h"
#include "./types/continue-inventory-response.h"

/**
 * Class for message parser
 */ 
class MessageParser {

  private:

    const static uint32_t firstStartMarker = 0xA5;
    const static uint32_t secondStartMarker = 0x5A;

    const static uint32_t firstEndMarker = 0x0D;
    const static uint32_t secondEndMarker = 0x0A;
    /**
     * Get message data length
     *
     * @param message antenna message
     * @return message data length
     */
    uint32_t getMessageDataLength(uint32_t message[]) {
      return (message[2] << 8) + (message[3]);
    }

  public:

    MessageParser() {};

    /**
     * Check message start values
     *
     * @param message antenna message
     * @return is message start valid or not
     */
    bool checkMessageStart(uint32_t message[]) {
      return (
        message[0] == firstStartMarker &&
        message[1] == secondStartMarker
      );
    }

    /**
     * Check message end values
     *
     * @param message antenna message
     * @param messageLength length of the message
     * @return is message end valid or not
     */
    bool checkMessageEnd(uint32_t message[], uint32_t messageLength) {
      return (
        message[messageLength -2] == firstEndMarker &&
        message[messageLength -1] == secondEndMarker
      );
    }

    /**
     * Check message CRC value
     *
     * @param message antenna message
     * @return is calculated CRC valid or not
     */
    bool checkCRC(uint32_t message[]) {
      const int messageDataLength = getMessageDataLength(message);
      int result = 0;

      for (int i = 0; i < messageDataLength; i++) {
        if (i > 1 && i < messageDataLength -3) {
          result ^= message[i];
        }
      }

      return (result == message[messageDataLength - 3]);
    }

    /**
     * Construct hex string from hex decimals.
     * Example:
     * Input: 0xE2, 0x00, 0x34, 0x11, 0xB8, 0x02, 0x01, 0x13, 0x83, 0x25, 0x85, 0x66
     * Output: E2003411B802011383258566
     *
     * @param message antenna message
     * @param start start index
     * @param end end index
     * @return string of hex values
     */
    std::string constructHexString(uint32_t message[], int start, int end) {
      std::stringstream buffer;

      for (int i = start; i < end; i++) {
        buffer << std::hex << std::setw(2) << std::setfill('0') << message[i];
      }
      buffer << std::hex << std::setw(2) << std::setfill('0') << message[end];
      return buffer.str();
    }

    /**
     * Convert hex string to int.
     * Example:
     * Input: 0DF732
     * Output: 915250
     *
     * @param value string value
     * @return converted int value
     */
    int hexStringToInt(std::string value) {
      unsigned int x;   
      std::stringstream ss;
      ss << std::hex << value;
      ss >> x;
      return x;
    }

    /**
     * Get twos complement
     *
     * @param value string value
     * @return double value of complement
     */
    double twosComplement(std::string value) {
      return (~hexStringToInt(value) + 1);
    }

    /**
     * Transforms RSSI to signal strength
     *
     * @param rssi RSSI value
     * @return calculated signal strength
     */
    double transformRssiToSignalStrength(double rssi) {
      int PdBmMax = -30;
      int PdBmMin = -80;
      return 100 * (1 - (PdBmMax - rssi) / (PdBmMax - PdBmMin));
    }

    /**
     * Parses continue inventory response message
     *
     * @param message antenna message
     */
    ContinueInventoryMessage parseContinueInventoryResponse(uint32_t message[]) {

      std::string pc = constructHexString(message, 5, 6);
      std::string epc = constructHexString(message, 7, 18);
      std::string rssiString = constructHexString(message, 19, 20);
      uint16_t rssi = twosComplement(rssiString);
      std::string frequency = constructHexString(message, 22, 24);
      std::string phase = constructHexString(message, 25, 25);

      ContinueInventoryMessage responseMessage(
        message[4],
        pc,
        epc,
        transformRssiToSignalStrength(-rssi / 10.0),
        message[21],
        hexStringToInt(frequency) / 1000.00,
        message[25]
      );
      return responseMessage;
    }

    /**
     * Parses continue inventory response message
     *
     * @param message antenna message
     */
    bool parseStopContinueInventoryResponse(uint32_t message[]) {
      return ( message[5] == 0x01);
    }
};