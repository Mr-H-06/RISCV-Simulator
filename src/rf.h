#ifndef RF_H
#define RF_H
#include <cstdint>

class RegisterFile {
  uint32_t reg[32];
  uint32_t rename_map[32];
};
#endif //RF_H
