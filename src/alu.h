#ifndef ALU_H
#define ALU_H
#include "returnlib.h"

struct ALUReturn {
  ALUReturn() = default;
};
class ALU {
public:
  ALU() {
    emtpy = true;
    time = 0;
  }
  ALUReturn run(RSEntry &entry) {
    if (emtpy) return ALUReturn();

    if (entry.opcode == ADD) {

    }
  }
  bool emtpy;
  DecodedIns code;
  uint32_t time;
};

#endif //ALU_H
