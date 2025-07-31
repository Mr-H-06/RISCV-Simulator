#ifndef ROB_H
#define ROB_H
#include <complex.h>

#include "rf.h"
#include "returnlib.h"

class ReorderBuffer {
public:
  bool regDependencyCheck(uint32_t reg_idx, uint32_t dest) {
    if (rf.rat[reg_idx] != dest || rf.rat[reg_idx] == -1 ||) {
      return true;
    }
    return false;
  }

  ReorderBuffer run(DecodedIns ins, RSReturn &rsret, LSBReturn &lsbret, RoBReturn &ret) {
    ReorderBuffer next = *this;
    next.push(ins);
    if (lsbret.add) {
      next.rob[lsbret.add_id].state = RoBEntry::Executing;
    }
    if (lsbret.pop) {
      next.rob[lsbret.rob_id].state = RoBEntry::Writeback;
      if (next.rob[])
    }

    next.pop();
  }
  void push(DecodedIns &decoded_ins) {
    if (decoded_ins.opcode == INVALID) {
      return;
    }
    uint32_t t = rear;
    rob[rear].busy = true;
    rob[rear].instruction = decoded_ins;
    rob[rear].state = RoBEntry::Issued;
    rob[rear].dest = decoded_ins.rd;
    rob[rear].value = 0;
    rob[rear].pc = decoded_ins.pc;
    if (rob[rear].dest)
    rear = (rear + 1) % Num;
  }

  void pop() {
    if (head == rear || rob[head].state != RoBEntry::Committed) {
      return;
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
