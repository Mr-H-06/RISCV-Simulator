#ifndef ALU_H
#define ALU_H
#include "returnlib.h"

class ALU {
public:
  ALUReturn run(RSEntry &entry) {
    ALUReturn ret = ALUReturn();
    Opcode opcode = entry.opcode;
    if (opcode == ADD || opcode == ADDI) {
      ret.data = entry.vj + entry.vk;
    } else if (opcode == SUB) {
      ret.data = entry.vj - entry.vk;
    } else if (opcode == AND || opcode == ANDI) {
      ret.data = entry.vj & entry.vk;
    } else if (opcode == OR || opcode == ORI) {
      ret.data = entry.vj | entry.vk;
    } else if (opcode == XOR || opcode == XORI) {
      ret.data = entry.vj ^ entry.vk;
    } else if (opcode == SLL || opcode == SLLI) {
      ret.data = entry.vj << (entry.vk & 0x1F);
    } else if (opcode == SRL || opcode == SRLI) {
      ret.data = entry.vj >> (entry.vk & 0x1F);
    } else if (opcode == SRA || opcode == SRAI) {
      ret.data = (int32_t) entry.vj >> (entry.vk & 0x1F);
    } else if (opcode == SLT || opcode == SLTI) {
      ret.data = ((int32_t) entry.vj < (int32_t) entry.vk) ? 1 : 0;
    } else if (opcode == SLTU || opcode == SLTIU) {
      ret.data = (entry.vj < entry.vk) ? 1 : 0;
    } else if (opcode == LUI) {
      ret.data = entry.vj;
    } else if (opcode == AUIPC) {
      ret.data = entry.pc + entry.vj;
    } else if (opcode == JAL) {
      ret.data = entry.pc + 4;
      ret.branch = true;
      ret.pc_to = entry.pc + entry.vj;
    } else if (opcode == JALR) {
      ret.data = entry.pc + 4;
      ret.branch = true;
      ret.pc_to = (entry.vj + entry.vk) & (~1);
    } else {
      if (opcode ==BEQ) {
        ret.branch = (entry.vj == entry.vk);
      } else if (opcode == BNE) {
        ret.branch = (entry.vj != entry.vk);
      } else if (opcode == BLT) {
        ret.branch = ((int32_t) entry.vj < (int32_t)entry.vk);
      } else if (opcode == BGE) {
        ret.branch = ((int32_t) entry.vj >= (int32_t) entry.vk);
      } else if (opcode == BLTU) {
        ret.branch = ((uint32_t)entry.vj < (uint32_t)entry.vk);
      } else if (opcode == BGEU) {
        ret.branch = ((uint32_t)entry.vj >= (uint32_t)entry.vk);
      }
      if (ret.branch) {
        ret.pc_to = entry.pc + entry.A;
      }
    }
    return ret;
  }
};

#endif //ALU_H
