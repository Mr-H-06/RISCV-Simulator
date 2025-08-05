#ifndef SIMULATOR_H
#define SIMULATOR_H
#include "returnlib.h"
#include "memory.h"
#include "predictor.h"
#include "rs.h"
#include "rob.h"
#include "lsb.h"

class Simulator {
public:
  Simulator(std::string &filename) : memory(filename), pc(0), pc_next(0), decoded_entry(), decoded_entry_next(), rob(),
                                     rob_next(), robret(), rs(), rs_next(), rsret(), rsret_next(), lsb(),
                                     lsb_next(), lsbret(), lsbret_next() {
  }

  Simulator() : memory(), pc(0), pc_next(0), decoded_entry(), decoded_entry_next(), rob(),
                rob_next(), robret(), rs(), rs_next(), rsret(), rsret_next(), lsb(),
                lsb_next(), lsbret(), lsbret_next() {
  }

  void run() {
    uint32_t cycle;
    for (cycle = 0; cycle < 2e10; cycle++) {
      transfer();
      fetch();
      rob_next = rob.run(decoded_entry, rsret, lsbret, robret, memory);
      rs_next = rs.run(rsret, rsret_next, rob);
      lsb_next = lsb.run(memory, lsbret_next, rob);
      if (robret.pc_jump) {
        pc_next = robret.pc;
        rob_next.clear();
        rs_next = ReservationStation();
        lsb_next = LoadStoreBuffer();
        DecodedIns d = DecodedIns();
        d.opcode = INVALID;
        d.opcode_type = INV;
        decoded_entry_next = d;
      } else {
        if (decoded_entry_next.opcode != INVALID) {
          pc_next = predictor(pc, decoded_entry);
        }
      }
      if (robret.exit) {
        std::cout << robret.exit_num << '\n';
        break;
      }
      //std::cerr << cycle << ' ' << pc << '\n';
    }
    //std::cerr << cycle << '\n';
  }

private:
  int32_t sign_extend(uint32_t value, int bits) {
    return (int32_t) (value << (32 - bits)) >> (32 - bits);
  }

  void transfer() {
    pc = pc_next;
    rob = rob_next;
    decoded_entry = decoded_entry_next;

    rs = rs_next;
    rsret = rsret_next;
    lsb = lsb_next;
    lsbret = lsbret_next;
  }

  void fetch() {
    // in: pc, out: decoded_entry
    uint32_t fetchIns;
    if (rob.willfull()) {
      DecodedIns ret = DecodedIns();
      ret.opcode = INVALID;
      ret.opcode_type = INV;
      decoded_entry_next = ret;
      return;
    }
    fetchIns = memory.get(pc);
    fetchIns += memory.get(pc + 1) << 8;
    fetchIns += memory.get(pc + 2) << 16;
    fetchIns += memory.get(pc + 3) << 24;
    decoded_entry_next = decode(fetchIns);
  }

  DecodedIns decode(uint32_t fetchIns) {
    DecodedIns decoded_ins = DecodedIns();
    decoded_ins.pc = pc;
    if (fetchIns == 0x0ff00513) {
      decoded_ins.opcode = EXIT;
      decoded_ins.opcode_type = EX;
      decoded_ins.rd = 0;
      decoded_ins.imm = 0;
      decoded_ins.pc = 0;
      decoded_ins.rs1 = 0;
      decoded_ins.rs2 = 0;
      return decoded_ins;
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
      decoded_ins.opcode = AUIPC;
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
      decoded_ins.imm = sign_extend(decoded_ins.imm, 21);
    } else if (type == 0b1100111) {
      //I_2 -- JALR
      decoded_ins.opcode_type = I2;
      decoded_ins.opcode = JALR;
      decoded_ins.rd = (fetchIns & 0b11111);
      fetchIns >>= 5;
      if ((fetchIns & 0b111) != 0b000) {
        decoded_ins.opcode = INVALID;
      }
      fetchIns >>= 3;
      decoded_ins.rs1 = (fetchIns & 0b11111);
      fetchIns >>= 5;
      decoded_ins.imm = fetchIns;
      decoded_ins.imm = sign_extend(decoded_ins.imm, 12);
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
      decoded_ins.imm = sign_extend(decoded_ins.imm, 13);
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
      decoded_ins.imm = sign_extend(decoded_ins.imm, 12);
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
      decoded_ins.imm = sign_extend(decoded_ins.imm, 12);
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
        decoded_ins.imm = (fetchIns & 0b11111);
        fetchIns >>= 5;
        if (fetchIns == 0b0000000) {
          decoded_ins.opcode = SLLI;
        } else {
          decoded_ins.opcode = INVALID;
        }
        return decoded_ins;
      } else if (type == 0b101) {
        decoded_ins.imm = (fetchIns & 0b11111);
        fetchIns >>= 5;
        if (fetchIns == 0b0000000) {
          decoded_ins.opcode = SRLI;
        } else if (fetchIns == 0b0100000) {
          decoded_ins.opcode = SRAI;
        } else {
          decoded_ins.opcode = INVALID;
        }
        return decoded_ins;
      } else {
        decoded_ins.opcode = INVALID;
      }
      decoded_ins.imm = fetchIns;
      decoded_ins.imm = sign_extend(decoded_ins.imm, 12);
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
    return decoded_ins;
  }

  Memory memory; //包括指令和内存
  uint32_t pc;
  uint32_t pc_next;
  DecodedIns decoded_entry;
  DecodedIns decoded_entry_next;

  ReorderBuffer rob;
  ReorderBuffer rob_next;
  RoBReturn robret;

  ReservationStation rs;
  ReservationStation rs_next;
  RSReturn rsret;
  RSReturn rsret_next;

  LoadStoreBuffer lsb;
  LoadStoreBuffer lsb_next;
  LSBReturn lsbret;
  LSBReturn lsbret_next;
};

#endif //SIMULATOR_H
