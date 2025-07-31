#ifndef RS_H
#define RS_H
#include "rob.h"
#include "rf.h"
#include "alu.h"


class ReservationStation {
public:
  ReservationStation() = default;

  void push(uint32_t &dest, DecodedIns &decoded_ins) {
    queue[rear].busy = false;
    queue[rear].opcode = decoded_ins.opcode;
    queue[rear].qj = -1;
    queue[rear].qk = -1;
    if (decoded_ins.opcode_type == R) {
      queue[rear].vj = decoded_ins.rs1;
      queue[rear].qj =
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

  ReservationStation run(RSReturn &rsret, RoBReturn &robret) {

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

  RSEntry queue[Num];
  uint32_t head;
  uint32_t rear;

  ALU alu;
  DecodedIns alu_in;
};
#endif //RS_H
