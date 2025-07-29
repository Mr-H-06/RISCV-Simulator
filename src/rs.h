#ifndef RS_H
#define RS_H
#include "rob.h"

template<uint32_t Num>
class ReservationStation {
public:
  struct RSEntry {
    bool busy;
    Opcode opcode;
    int32_t vj;
    int32_t vk;
    uint32_t qj;
    uint32_t qk;
    uint32_t dest;
    uint32_t A;
  };
  ReservationStation() = default;

  void push(uint32_t &dest, DecodedIns &decoded_ins) {
    queue[rear].busy = false;
    queue[rear].opcode = decoded_ins.opcode;
    if (decoded_ins.opcode_type == R) {
      queue[rear].vj = decoded_ins.rs1;
      queue[rear].vk = decoded_ins.rs2;
    } else if (decoded_ins.opcode_type == I2) {
      queue[rear].vj = decoded_ins.rs1;
      queue[rear].vk = decoded_ins.imm;
    } else if (decoded_ins.opcode_type == B) {
    } else if (decoded_ins.opcode_type == J) {
    } else if (decoded_ins.opcode_type == U) {
    }
    queue[rear].dest = dest;
    ++rear;
  }

  RSEntry get() {
    for (uint32_t ins = head; ins != rear; ins = (ins + 1) % Num) {
      if (queue[ins].qj == 0 && queue[ins].qk == 0) {
        RSEntry ret = queue[ins];
        for (uint32_t i = ins; (i + 1) % Num != rear; i = (i + 1) % Num) {
          queue[i] = queue[(i + 1) % Num];
        }
        return ret;
      }
    }
    RSEntry ret;
    ret.opcode = INVALID;
    return ret;
  }

  void pop() {
    ++head;
  }

private:
  RSEntry queue[Num];
  uint32_t head;
  uint32_t rear;
};
#endif //RS_H
