#ifndef PREDICTOR_H
#define PREDICTOR_H
#include <cstdint>
uint32_t predictor(uint32_t pc, DecodedIns decoded_entry) {
  return pc + 4;
}
#endif //PREDICTOR_H
