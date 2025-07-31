#ifndef RF_H
#define RF_H
#include <cstdint>
class RegisterFile {
public:
  RegisterFile() {
    for (uint32_t i = 0; i < 32; ++i) {
      reg[i] = 0;
      rat[i] = -1;
    }
  }
  uint32_t read(uint32_t idx) {
    return reg[idx];
  }

  void write(uint32_t idx, uint32_t value) {
    reg[idx] = value;
  }

  uint32_t reg[32];
  int32_t rat[32];
};
#endif //RF_H
