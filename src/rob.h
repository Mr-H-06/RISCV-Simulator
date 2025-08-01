#ifndef ROB_H
#define ROB_H
#include "rf.h"
#include "returnlib.h"

class ReorderBuffer {
public:
  ReorderBuffer() {
    head = 0;
    rear = 0;
  }

  bool regDependencyCheck(uint32_t reg_idx, uint32_t dest) {
    if (rf.rat[reg_idx] == -1 || !rob[rf.rat[reg_idx]].ready || rf.rat[reg_idx] != dest) {
      return true;
    }
    return false;
  }

  void push(DecodedIns &decoded_ins) {
    if (decoded_ins.opcode == INVALID) {
      return;
    }
    rob[rear].prepared = false;
    rob[rear].ready = false;
    rob[rear].instruction = decoded_ins;
    rob[rear].state = RoBEntry::Issued;
    rob[rear].value = 0;
    rob[rear].pc = decoded_ins.pc;
    if (!regDependencyCheck(decoded_ins.rs1, rear)) {
      rob[rear].rs1val = rf.reg[decoded_ins.rs1];
    }
    if (!regDependencyCheck(decoded_ins.rs2, rear)) {
      rob[rear].rs2val = rf.reg[decoded_ins.rs2];
    }
    if (decoded_ins.rd != 0) {
      rf.rat[decoded_ins.rd] = rear;
    }
    rear = (rear + 1) % Num;
  }

  ReorderBuffer run(DecodedIns ins, RSReturn &rsret, LSBReturn &lsbret, RoBReturn &ret) {
    rf.rat[0] = -1;
    rf.reg[0] = 0;
    ReorderBuffer next = *this;
    next.push(ins);
    if (lsbret.add) {
      next.rob[lsbret.add_id].state = RoBEntry::Executing;
    }
    if (lsbret.pop) {
      next.rob[lsbret.rob_id].state = RoBEntry::Writeback;
      next.rob[lsbret.rob_id].ready = true;
      next.rob[lsbret.rob_id].value = lsbret.data;
    }
    if (rsret.add) {
      next.rob[rsret.add_id].state = RoBEntry::Executing;
    }
    if (rsret.pop) {
      next.rob[rsret.rob_id].state = RoBEntry::Writeback;
      next.rob[rsret.rob_id].ready = true;
      next.rob[rsret.rob_id].value = rsret.aluret.data;
      if (rsret.aluret.branch) {
        ret.pc_jump = true;
        ret.pc = rsret.aluret.pc_to;
      }
    }

    next.pop();
    for (uint32_t ins = next.head; ins != next.rear; ins = (ins + 1) % Num) {
      if (!next.regDependencyCheck(next.rob[ins].instruction.rs1, ins) && !next.regDependencyCheck(next.rob[ins].instruction.rs2, ins)) {
        next.rob[ins].ready = true;
      } else {
        next.rob[ins].ready = false;
      }
    }
  }

  void pop() {
    if (head == rear || rob[head].state != RoBEntry::Writeback) {
      return;
    }
    rob[head].state = RoBEntry::Committed;
    rob[head].ready = false;
    if (rob[head].instruction.rd != 0) {
      if (rf.rat[rob[head].instruction.rd] == head) {
        rf.rat[rob[head].instruction.rd] = -1;
        rf.reg[rob[head].instruction.rd] = rob[head].value;
      }
    }
    head = (head + 1) % Num;
  }

  bool full() {
    return (rear + 1) % Num == head;
  }

  uint32_t getreg(uint32_t reg_idx) {
    if (rf.rat[reg_idx] != -1) {
      return rob[rf.rat[reg_idx]].value;
    }
    return rf.reg[reg_idx];
  }

  RoBEntry rob[Num];
  RegisterFile rf;
  uint32_t head;
  uint32_t rear;
};
#endif //ROB_H
