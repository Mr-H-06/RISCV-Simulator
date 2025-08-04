#ifndef RS_H
#define RS_H
#include "rob.h"
#include "rf.h"
#include "alu.h"


class ReservationStation {
public:
  ReservationStation() = default;

  void push(uint32_t dest, DecodedIns &decoded_ins, ReorderBuffer &rob) {
    queue[size].opcode = decoded_ins.opcode;
    queue[size].pc = rob.rob[dest].pc;
    if (decoded_ins.opcode_type == R) {
      queue[size].vj = rob.rob[dest].rs1val;
      queue[size].vk = rob.rob[dest].rs2val;
    } else if (decoded_ins.opcode_type == I2) {
      queue[size].vj = rob.rob[dest].rs1val;
      queue[size].vk = decoded_ins.imm;
    } else if (decoded_ins.opcode_type == B) {
      queue[size].vj = rob.rob[dest].rs1val;
      queue[size].vk = rob.rob[dest].rs2val;
      queue[size].A = decoded_ins.imm;
    } else if (decoded_ins.opcode_type == J) {
      queue[size].vj = decoded_ins.imm;
    } else if (decoded_ins.opcode_type == U) {
      queue[size].vj = decoded_ins.imm;
    }
    queue[size].dest = dest;
    ++size;
  }

  ReservationStation run(RSReturn &ret_last, RSReturn &ret, ReorderBuffer &rob) {
    ret = RSReturn();
    ReservationStation next = *this;
    ret.add = false;
    ret.pop = false;
    if (!next.full()) {
      for (int32_t i = rob.head; i != rob.rear; i = (i + 1) % Num) {
        OpcodeType type = rob.rob[i].instruction.opcode_type;
        if (type == U || type == J || type == I2 || type == B || type == R) {
          if (rob.rob[i].prepared && rob.rob[i].state == RoBEntry::Issued) {
            if (ret_last.add && ret_last.add_id == i) continue;
            next.push(i, rob.rob[i].instruction, rob);
            ret.add = true;
            ret.add_id = i;
            break;
          }
        }
      }
    }
    // alu working
    if (!empty()) {
      ret.pop = true;
      ret.rob_id = alu_in.dest;
      ret.aluret = alu.run(alu_in);
      for (int32_t i = 0; i < next.size - 1; ++i) {
        next.queue[i] = next.queue[i + 1];
      }
      --next.size;
    }
    if (!next.empty()) {
      next.alu_in = next.queue[0];
    }
    return next;
  }

  bool empty() {
    return size == 0;
  }

  bool full() {
    return size == Num;
  }

  RSEntry queue[Num];
  int32_t size;

  ALU alu;
  RSEntry alu_in;
};
#endif //RS_H
