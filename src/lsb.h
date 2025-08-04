#ifndef LSB_H
#define LSB_H
#include "returnlib.h"
#include "rob.h"
#include "memory.h"

class LoadStoreBuffer {
public:
  LoadStoreBuffer() : size(0), working(false), working_idx(0) {
  }

  void clear() {
    size = 0;
  }

  void push(uint32_t dest, DecodedIns &decoded_ins, ReorderBuffer &rob) {
    queue[size].opcode = decoded_ins.opcode;
    queue[size].rob_id = dest;
    queue[size].rs1val = rob.rob[dest].rs1val;
    queue[size].rs2val = rob.rob[dest].rs2val;
    queue[size].imm = decoded_ins.imm;
    queue[size].addr = rob.rob[dest].rs1val + decoded_ins.imm;
    if (decoded_ins.opcode_type == I1) {
      queue[size].is_load = true;
    } else {
      queue[size].is_load = false;
    }
    queue[size].time = 3;
    ++size;
  }

  LoadStoreBuffer run(Memory &memory, LSBReturn &ret_last, LSBReturn &ret, ReorderBuffer &rob) {
    ret = LSBReturn();
    LoadStoreBuffer next = *this;
    ret.add = false;
    ret.pop = false;
    if (!next.full()) {
      for (int32_t i = rob.head; i != rob.rear; i = (i + 1) % Num) {
        if (rob.rob[i].instruction.opcode_type == I1 || rob.rob[i].instruction.opcode_type == S) {
          if (rob.rob[i].prepared && rob.rob[i].state == RoBEntry::Issued) {
            if (!(ret_last.add && ret_last.add_id == i)){
              next.push(i, rob.rob[i].instruction, rob);
              ret.add = true;
              ret.add_id = i;
              break;
            }
          }
          if (rob.rob[i].instruction.opcode_type == S) {
            break;
          }
        }
      }
    }
    if (empty()) return next;
    if (next.working) {
      --next.queue[working_idx].time;
      if (next.queue[working_idx].time == 0) {
        uint32_t addr = next.queue[working_idx].addr;
        if (next.queue[working_idx].is_load) {
          if (next.queue[working_idx].opcode == LB) {
            next.queue[working_idx].data = int32_t(memory.get(addr));
          } else if (next.queue[working_idx].opcode == LBU) {
            next.queue[working_idx].data = uint32_t(memory.get(addr));
          } else if (next.queue[working_idx].opcode == LH) {
            next.queue[working_idx].data = int32_t(memory.get(addr) | (memory.get(addr + 1) << 8));
          } else if (next.queue[working_idx].opcode == LHU) {
            next.queue[working_idx].data = uint32_t(
              memory.get(addr) | (memory.get(addr + 1) << 8));
          } else if (next.queue[working_idx].opcode == LW) {
            next.queue[working_idx].data = uint32_t(
              memory.get(addr) | (memory.get(addr + 1) << 8) | (memory.get(addr + 2) << 16) | (
                memory.get(addr + 3) << 24));
          }
        } else {
          uint32_t data = next.queue[working_idx].rs2val;
          next.queue[working_idx].data = data;
          /*
          if (next.queue[working_idx].opcode == SB) {
            memory.write(next.queue[working_idx].addr, data & 0xFF);
          } else if (next.queue[working_idx].opcode == SH) {
            memory.write(next.queue[working_idx].addr, data & 0xFF);
            memory.write(next.queue[working_idx].addr + 1, (data >> 8) & 0xFF);
          } else if (next.queue[working_idx].opcode == SW) {
            memory.write(next.queue[working_idx].addr, data & 0xFF);
            memory.write(next.queue[working_idx].addr + 1, (data >> 8) & 0xFF);
            memory.write(next.queue[working_idx].addr + 2, (data >> 16) & 0xFF);
            memory.write(next.queue[working_idx].addr + 3, (data >> 24) & 0xFF);
          }*/
        }
        ret.pop = true;
        ret.rob_id = next.queue[working_idx].rob_id;
        ret.data = next.queue[working_idx].data;
        //ret.is_load = next.queue[working_idx].is_load;
        for (int32_t k = working_idx; k < size - 1; ++k) {
          next.queue[k] = next.queue[k + 1];
        }
        next.working = false;
        --next.size;
      }
    } else {
      //uint32_t id = next.queue[0].rob_id;
      if (next.queue[0].is_load) {
        //if (rob.regDependencyCheck(rob.rob[id].instruction.rd, id) && rob.regDependencyCheck(next.queue[0].rs1, id)) {
        next.working_idx = 0;
        next.working = true;
        //next.queue[0].addr = next.queue[0].rs1val + next.queue[0].imm;
        --next.queue[0].time;
        //}
      } else {
        //if (rob.regDependencyCheck(next.queue[0].rs1, id) && rob.regDependencyCheck(next.queue[0].rs2, id)) {
        next.working_idx = 0;
        next.working = true;
        //next.queue[0].addr = next.queue[0].rs1val + next.queue[0].imm;
        --next.queue[0].time;
        //}
      }
    }
    return next;
  }

  bool full() {
    return size == Num;
  }

  bool empty() {
    return size == 0;
  }

private:
  struct LSBEntry {
    bool is_load;
    Opcode opcode;
    uint32_t addr;
    uint32_t rs1val;
    uint32_t rs2val;
    uint32_t imm;
    uint32_t data;
    uint32_t rob_id;
    uint32_t time;
  };

  LSBEntry queue[Num];
  int32_t size;

  bool working;
  uint32_t working_idx;
};
#endif //LSB_H
