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

  bool regDependencyCheck(int32_t reg_idx, int32_t dest) {
    if (rf.rat[reg_idx] == -1 || rob[rf.rat[reg_idx]].ready || rf.rat[reg_idx] == dest) {
      return false;
    }
    return true;
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
    rob[rear].jump = false;
    rob[rear].jump_pc = 0;
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
    if (decoded_ins.opcode == EXIT) {
      rob[rear].state = RoBEntry::Writeback;
    }
  }

  ReorderBuffer run(DecodedIns ins, RSReturn &rsret, LSBReturn &lsbret, RoBReturn &ret, Memory &memory) {
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
        next.rob[rsret.rob_id].jump = true;
        next.rob[rsret.rob_id].jump_pc = rsret.aluret.pc_to;
      }
    }

    next.commit(next, ret, memory);
    for (int32_t i = next.head; i != next.rear; i = (i + 1) % Num) {
      if (!next.regDependencyCheck(next.rob[i].instruction.rs1, i) && !next.regDependencyCheck(
            next.rob[i].instruction.rs2, i)) {
        next.rob[i].prepared = true;
      } else {
        next.rob[i].prepared = false;
      }
    }
    return next;
  }

  void commit(ReorderBuffer &next, RoBReturn &ret, Memory &memory) {
    if (next.head == next.rear || next.rob[head].state != RoBEntry::Writeback) {
      return;
    }
    if (next.rob[head].instruction.opcode == EXIT) {
      ret.exit = true;
      ret.exit_num = rf.reg[10] & 0xFF;
      return;
    }
    next.rob[next.head].state = RoBEntry::Committed;
    next.rob[next.head].ready = false;
    /*if (next.rob[next.head].instruction.opcode_type == I1) {
      uint32_t addr = next.rob[next.head].rs1val + next.rob[next.head].instruction.imm;
      if (next.rob[next.head].instruction.opcode == LB) {
        next.rob[next.head].value = int32_t(memory.get(addr));
      } else if (next.rob[next.head].instruction.opcode == LBU) {
        next.rob[next.head].value = uint32_t(memory.get(addr));
      } else if (next.rob[next.head].instruction.opcode == LH) {
            next.rob[next.head].value = int32_t(memory.get(addr) | (memory.get(addr + 1) << 8));
      } else if (next.rob[next.head].instruction.opcode == LHU) {
        next.rob[next.head].value = uint32_t(
          memory.get(addr) | (memory.get(addr + 1) << 8));
      } else if (next.rob[next.head].instruction.opcode == LW) {
        next.rob[next.head].value = uint32_t(
          memory.get(addr) | (memory.get(addr + 1) << 8) | (memory.get(addr + 2) << 16) | (
            memory.get(addr + 3) << 24));
      }
    } else */if (next.rob[head].instruction.opcode_type == S) {
      uint32_t data = next.rob[next.head].rs2val;
      uint32_t addr = next.rob[next.head].rs1val + next.rob[next.head].instruction.imm;
      next.rob[next.head].value = data;
      if (next.rob[next.head].instruction.opcode == SB) {
        memory.write(addr, data & 0xFF);
      } else if (next.rob[next.head].instruction.opcode == SH) {
        memory.write(addr, data & 0xFF);
        memory.write(addr + 1, (data >> 8) & 0xFF);
      } else if (next.rob[next.head].instruction.opcode == SW) {
        memory.write(addr, data & 0xFF);
        memory.write(addr + 1, (data >> 8) & 0xFF);
        memory.write(addr + 2, (data >> 16) & 0xFF);
        memory.write(addr + 3, (data >> 24) & 0xFF);
      }
    }
    if (next.rob[next.head].instruction.rd != 0) {
      if (next.rf.rat[rob[next.head].instruction.rd] == head) {
        rf.rat[next.rob[next.head].instruction.rd] = -1;
        rf.reg[next.rob[next.head].instruction.rd] = next.rob[next.head].value;
      }
    }
    if (next.rob[next.head].jump) {
      ret.pc_jump = true;
      ret.pc = rob[head].jump_pc;
      next.rear = (next.head + 1) % Num;
    } else {
      ret.pc_jump = false;
    }
    next.head = (next.head + 1) % Num;
  }

  bool willfull() {
    return (rear + 2) % Num == head || (rear + 1) % Num == head;
  }

  uint32_t getreg(uint32_t reg_idx) {
    if (rf.rat[reg_idx] != -1) {
      return rob[rf.rat[reg_idx]].value;
    }
    return rf.reg[reg_idx];
  }

  void clear() {
    head = 0;
    rear = 0;
  }

  RoBEntry rob[Num];
  RegisterFile rf;
  int32_t head;
  int32_t rear;
};
#endif //ROB_H
