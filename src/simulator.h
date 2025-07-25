#ifndef SIMULATOR_H
#define SIMULATOR_H
#include <memory.h>

enum Opcode : uint8_t {
  LUI, AUIIPC, JAL, JALR, BEQ, BNE, BLT, BGE, BLTU, BGEU,
  LB, LH, LW, LBU, LHU, SB, SH, SW, ADDI, SLTI,
  SLTIU, XORI, ORI, ANDI, SLLI, SRLI, SRAI, ADD, SUB, SLL,
  SLT, SLTU, XOR, SRL, SRA, OR, AND,
  INVALID
};

class Simulator {
public:
  Simulator(std::string &filename) : memory_(filename), pc_(0), fetchIns(0) {
  }

  Simulator() : memory_(), pc_(0), fetchIns(0) {
  }

  void run() {
    uint32_t cycle;
    for (cycle = 0; ; cycle++) {
      fetch();
      execute();
      writeback();
      if (pc_) {
        break;
      }
    }
    std::cout << '\n' << cycle;
  }

private:
  void fetch() {
    fetchIns = memory_.getIns(pc_);
    fetchIns += memory_.getIns(pc_ + 1) << 8;
    fetchIns += memory_.getIns(pc_ + 2) << 16;
    fetchIns += memory_.getIns(pc_ + 3) << 24;
    decode();
  }

  void decode() {
    uint32_t type = fetchIns & 111111;
    if (type == 0b0110111) {

    } else if (type == 0b0010111) {

    } else if (type == 0b1101111) {

    } else if (type == 0b1100111) {

    } else if (type == 0b1100011) {

    } else if (type == 0b0000011) {

    } else if (type == 0b0100011) {

    } else if (type == 0b0010011) {

    } else if (type == 0b0110011) {

    } else if ()
  }

  void execute() {
  }

  void writeback() {
  }

  Memory memory_; //包括指令和内存
  uint32_t pc_;
  uint32_t fetchIns;
};

#endif //SIMULATOR_H
