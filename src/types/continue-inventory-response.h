#include <iostream>
using namespace std;

/**
 * Class for continue inventory response message
 */
class ContinueInventoryMessage {
  private:
    uint32_t type;
    string pc;
    string epc;
    double strength;
    int16_t antenna;
    double frequency;
    int16_t phase;

  public:
    ContinueInventoryMessage(
      uint32_t type,
      string pc,
      string epc,
      double strength,
      int16_t antenna,
      double frequency,
      int16_t phase
    ) {
      setType(type);
      setPc(pc);
      setEpc(epc);
      setStrength(strength);
      setAntenna(antenna);
      setFrequency(frequency);
      setPhase(phase);
    };

    void setType(uint32_t _type) {
      type = _type;
    }

    void setPc(string _pc) {
      pc = _pc;
    }

    void setEpc(string _epc) {
      epc = _epc;
    }

    void setStrength(double _strength) {
      strength = _strength;
    }

    void setAntenna(int16_t _antenna) {
      antenna = _antenna;
    }

    void setFrequency(double _frequency) {
      frequency = _frequency;
    }

    void setPhase(int16_t _phase) {
      phase = _phase;
    }

    short getType() {
      return type;
    }

    string getPc() {
      return pc;
    }

    string getEpc() {
      return epc;
    }

    double getStrength() {
      return strength;
    }

    int16_t getAntenna() {
      return antenna;
    }

    double getFrequency() {
      return frequency;
    }

    int16_t getPhase() {
      return phase;
    }

    /**
     * Prints values of this object
     */
    void printValues() {
      std::cout << "Type: ";
      std::cout << type << std::endl;
      std::cout << "PC: ";
      std::cout << pc << std::endl;
      std::cout << "EPC: ";
      std::cout << epc << std::endl;
      std::cout << "Strength: ";
      std::cout << strength << std::endl;
      std::cout << "Antenna: ";
      std::cout << antenna << std::endl;
      std::cout << "Frequency: ";
      std::cout << frequency << std::endl;
      std::cout << "Phase: ";
      std::cout << phase << std::endl;
    }
};