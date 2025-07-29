#ifndef SIMULATOR_H
#define SIMULATOR_H
#include "memory.h"
#include "rs.h"
#include "alu.h"
#include "rob.h"
#include "rf.h"
#include "lsb.h"

class Simulator {
public:
  Simulator(std::string &filename) : memory(filename), pc(0), fetchIns(0) {
  }

  Simulator() : memory(), pc(0), fetchIns(0) {
  }

  void run() {
    uint32_t cycle;
    for (cycle = 0; ; cycle++) {
      fetch();
      issue();
      execute();
      write(); //including broadcast
      commit();
      if (pc) {
        break;
      }
    }
    std::cout << '\n' << cycle;
  }

private:
  void fetch() {
    fetchIns = memory.getIns(pc);
    fetchIns += memory.getIns(pc + 1) << 8;
    fetchIns += memory.getIns(pc + 2) << 16;
    fetchIns += memory.getIns(pc + 3) << 24;
    decode();
  }

  void decode() {
    decoded_ins.clear();
    if (fetchIns == 0x0ff00513) {
      decoded_ins.opcode = EXIT;
      return;
    }
    uint32_t type = fetchIns & 0b1111111;
    fetchIns >>= 7;
    if (type == 0b0110111) {
      decoded_ins.opcode_type = U;
      decoded_ins.opcode = LUI;
      decoded_ins.rd = (fetchIns & 0b11111);
      fetchIns >>= 5;
      decoded_ins.imm = fetchIns << 12;
    } else if (type == 0b0010111) {
      decoded_ins.opcode_type = U;
      decoded_ins.opcode = AUIIPC;
      decoded_ins.rd = (fetchIns & 0b11111);
      fetchIns >>= 5;
      decoded_ins.imm = fetchIns << 12;
    } else if (type == 0b1101111) {
      decoded_ins.opcode_type = J;
      decoded_ins.opcode = JAL;
      decoded_ins.rd = (fetchIns & 0b11111);
      fetchIns >>= 5;
      decoded_ins.imm = ((fetchIns & 0b11111111) << 12);
      fetchIns >>= 8;;
      decoded_ins.imm += ((fetchIns & 0b1) << 11);
      fetchIns >>= 1;
      decoded_ins.imm += ((fetchIns & 0b1111111111) << 1);
      fetchIns >>= 10;
      decoded_ins.imm += (fetchIns << 20);
    } else if (type == 0b1100111) {
      decoded_ins.opcode_type = I2;
      decoded_ins.opcode = JALR;
      decoded_ins.rd = (fetchIns & 0b11111);
      fetchIns >>= 5;
      if ((fetchIns & 111) != 0b000) {
        decoded_ins.opcode = INVALID;
      }
      fetchIns >>= 3;
      decoded_ins.rs1 = (fetchIns & 0b11111);
      fetchIns >>= 5;
      decoded_ins.imm = fetchIns;
    } else if (type == 0b1100011) {
      //B
      decoded_ins.opcode_type = B;
      decoded_ins.imm = 0;
      decoded_ins.imm += ((fetchIns & 0b1) << 11);
      fetchIns >>= 1;
      decoded_ins.imm += ((fetchIns & 0b1111) << 1);
      fetchIns >>= 4;
      type = fetchIns & 0b111;
      fetchIns >>= 3;
      if (type == 0b000) {
        decoded_ins.opcode = BEQ;
      } else if (type == 0b001) {
        decoded_ins.opcode = BNE;
      } else if (type == 0b100) {
        decoded_ins.opcode = BLT;
      } else if (type == 0b101) {
        decoded_ins.opcode = BGE;
      } else if (type == 0b110) {
        decoded_ins.opcode = BLTU;
      } else if (type == 0b111) {
        decoded_ins.opcode = BGEU;
      } else {
        decoded_ins.opcode = INVALID;
      }
      decoded_ins.rs1 = fetchIns & 0b11111;
      fetchIns >>= 5;
      decoded_ins.rs2 = fetchIns & 0b11111;
      fetchIns >>= 5;
      decoded_ins.imm += ((fetchIns & 0b111111) << 5);
      fetchIns >>= 6;
      decoded_ins.imm += ((fetchIns & 0b1) << 12);
    } else if (type == 0b0000011) {
      //I_1
      decoded_ins.opcode_type = I1;
      decoded_ins.rd = (fetchIns & 0b11111);
      fetchIns >>= 5;
      type = (fetchIns & 0b111);
      fetchIns >>= 3;
      if (type == 0b000) {
        decoded_ins.opcode = LB;
      } else if (type == 0b001) {
        decoded_ins.opcode = LH;
      } else if (type == 0b010) {
        decoded_ins.opcode = LW;
      } else if (type == 0b100) {
        decoded_ins.opcode = LBU;
      } else if (type == 0b101) {
        decoded_ins.opcode = LHU;
      } else {
        decoded_ins.opcode = INVALID;
      }
      decoded_ins.rs1 = (fetchIns & 0b11111);
      fetchIns >>= 5;
      decoded_ins.imm = fetchIns;
    } else if (type == 0b0100011) {
      //S
      decoded_ins.opcode_type = S;
      decoded_ins.imm = (fetchIns & 0b11111);
      fetchIns >>= 5;
      type = (fetchIns & 0b111);
      fetchIns >>= 3;
      if (type == 0b000) {
        decoded_ins.opcode = SB;
      } else if (type == 0b001) {
        decoded_ins.opcode = SH;
      } else if (type == 0b010) {
        decoded_ins.opcode = SW;
      } else {
        decoded_ins.opcode = INVALID;
      }
      decoded_ins.rs1 = (fetchIns & 0b11111);
      fetchIns >>= 5;
      decoded_ins.rs2 = (fetchIns & 0b11111);
      fetchIns >>= 5;
      decoded_ins.imm += (fetchIns << 5);
    } else if (type == 0b0010011) {
      //I_2
      decoded_ins.opcode_type = I2;
      decoded_ins.rd = (fetchIns & 0b11111);
      fetchIns >>= 5;
      type = (fetchIns & 0b111);
      fetchIns >>= 3;
      decoded_ins.rs1 = (fetchIns & 0b11111);
      fetchIns >>= 5;
      if (type == 0b000) {
        decoded_ins.opcode = ADDI;
      } else if (type == 0b010) {
        decoded_ins.opcode = SLTI;
      } else if (type == 0b011) {
        decoded_ins.opcode = SLTIU;
      } else if (type == 0b100) {
        decoded_ins.opcode = XORI;
      } else if (type == 0b110) {
        decoded_ins.opcode = ORI;
      } else if (type == 0b111) {
        decoded_ins.opcode = ANDI;
      } else if (type == 0b001) {
        decoded_ins.rs2 = (fetchIns & 0b11111);
        fetchIns >>= 5;
        if (fetchIns == 0b0000000) {
          decoded_ins.opcode = SLLI;
        } else {
          decoded_ins.opcode = INVALID;
        }
        return;
      } else if (type == 0b101) {
        decoded_ins.rs2 = (fetchIns & 0b11111);
        fetchIns >>= 5;
        if (fetchIns == 0b0000000) {
          decoded_ins.opcode = SRLI;
        } else if (fetchIns == 0b0100000) {
          decoded_ins.opcode = SRAI;
        } else {
          decoded_ins.opcode = INVALID;
        }
        return;
      } else {
        decoded_ins.opcode = INVALID;
      }
      decoded_ins.imm = fetchIns;
    } else if (type == 0b0110011) {
      //R
      decoded_ins.opcode_type = R;
      decoded_ins.rd = (fetchIns & 0b11111);
      fetchIns >>= 5;
      type = (fetchIns & 0b111);
      fetchIns >>= 3;
      decoded_ins.rs1 = (fetchIns & 0b11111);
      fetchIns >>= 5;
      decoded_ins.rs2 = (fetchIns & 0b11111);
      fetchIns >>= 5;
      if (fetchIns == 0b0000000) {
        if (type == 0b000) {
          decoded_ins.opcode = ADD;
        } else if (type == 0b001) {
          decoded_ins.opcode = SLL;
        } else if (type == 0b010) {
          decoded_ins.opcode = SLT;
        } else if (type == 0b011) {
          decoded_ins.opcode = SLTU;
        } else if (type == 0b100) {
          decoded_ins.opcode = XOR;
        } else if (type == 0b101) {
          decoded_ins.opcode = SRL;
        } else if (type == 0b110) {
          decoded_ins.opcode = OR;
        } else if (type == 0b111) {
          decoded_ins.opcode = AND;
        }
      } else if (fetchIns == 0b0100000) {
        if (type == 0b000) {
          decoded_ins.opcode = SUB;
        } else if (type == 0b101) {
          decoded_ins.opcode = SRA;
        } else {
          decoded_ins.opcode = INVALID;
        }
      } else {
        decoded_ins.opcode = INVALID;
      }
    } else {
      decoded_ins.opcode = INVALID;
    }
  }

  void issue() {
    /*if (decoded_ins.opcode == LUI) {
    } else if (decoded_ins.opcode == AUIIPC) {
    } else if (decoded_ins.opcode == JAL) {
    } else if (decoded_ins.opcode == JALR) {
    } else if (decoded_ins.opcode == BEQ) {
    } else if (decoded_ins.opcode == BNE) {
    } else if (decoded_ins.opcode == BLT) {
    } else if (decoded_ins.opcode == BGE) {
    } else if (decoded_ins.opcode == BLTU) {
    } else if (decoded_ins.opcode == BGEU) {
    } else if (decoded_ins.opcode == LB) {
    } else if (decoded_ins.opcode == LH) {
    } else if (decoded_ins.opcode == LW) {
    } else if (decoded_ins.opcode == LBU) {
    } else if (decoded_ins.opcode == LHU) {
    } else if (decoded_ins.opcode == SB) {
    } else if (decoded_ins.opcode == SH) {
    } else if (decoded_ins.opcode == SW) {
    } else if (decoded_ins.opcode == ADDI) {
    } else if (decoded_ins.opcode == SLTI) {
    } else if (decoded_ins.opcode == SLTIU) {
    } else if (decoded_ins.opcode == XORI) {
    } else if (decoded_ins.opcode == ORI) {
    } else if (decoded_ins.opcode == ANDI) {
    } else if (decoded_ins.opcode == SLLI) {
    } else if (decoded_ins.opcode == SRLI) {
    } else if (decoded_ins.opcode == SRAI) {
    } else if (decoded_ins.opcode == ADD) {
    } else if (decoded_ins.opcode == SUB) {
    } else if (decoded_ins.opcode == SLL) {
    } else if (decoded_ins.opcode == SLT) {
    } else if (decoded_ins.opcode == SLTU) {
    } else if (decoded_ins.opcode == XOR) {
    } else if (decoded_ins.opcode == SRL) {
    } else if (decoded_ins.opcode == SRA) {
    } else if (decoded_ins.opcode == OR) {
    } else if (decoded_ins.opcode == AND) {
    }*/
    if (decoded_ins.opcode != INVALID) {
      if (decoded_ins.opcode_type == I1 || decoded_ins.opcode_type == S) {
        uint32_t rob_id = rob.push(decoded_ins);
        lsb.push(rob_id, decoded_ins);
      } else {
        uint32_t rob_id = rob.push(decoded_ins);
        rs.push(rob_id, decoded_ins);
      }
    }

  }

  void execute() {

  }

  void write() {
  }

  void commit() {
  }

  ALU alu;
  ReorderBuffer<6> rob;
  DecodedIns decoded_ins;
  ReservationStation<5> rs;
  RegisterFile rf;
  LoadStoreBuffer<5> lsb;
  Memory memory; //包括指令和内存
  uint32_t pc;
  uint32_t fetchIns;
};

#endif //SIMULATOR_H
