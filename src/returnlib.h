#ifndef RETURNLIB_H
#define RETURNLIB_H
#include <cstdint>
constexpr uint32_t Num = 5;
//  fetch
enum Opcode : uint8_t {
  LUI, AUIIPC, JAL, JALR, BEQ, BNE, BLT, BGE, BLTU, BGEU,
  LB, LH, LW, LBU, LHU, SB, SH, SW, ADDI, SLTI,
  SLTIU, XORI, ORI, ANDI, SLLI, SRLI, SRAI, ADD, SUB, SLL,
  SLT, SLTU, XOR, SRL, SRA, OR, AND,
  INVALID, EXIT
};
enum OpcodeType : uint8_t {
  U, J, I1, I2, B, S, R
};
//  RoB
class DecodedIns {
public:
  DecodedIns() {
    clear();
  };

  void clear() {
    opcode = INVALID;
    rs1 = 0;
    rs2 = 0;
    rd = 0;
    imm = 0;
  }

  Opcode opcode;
  OpcodeType opcode_type;
  uint8_t rs1;
  uint8_t rs2; // = shamt when opcode = slli, srli, srai
  uint8_t rd;
  uint32_t imm;
  uint32_t pc;
};

struct RoBReturn {
  int32_t rob_idx;
  bool issue_ready;
  bool exit;
  uint32_t exit_num;
};

struct RoBEntry {
  enum State {
    Issued,
    Executing,
    Writeback,
    Committed,
  };

  bool busy;
  bool ready;
  DecodedIns instruction;
  State state;
  uint32_t dest; // reg/mem
  uint32_t value;
  uint32_t pc;
};


//  RS
struct RSEntry {
  RSEntry() = default;
  bool busy;
  Opcode opcode;
  int32_t vj;
  int32_t vk;
  int32_t qj;
  int32_t qk;
  uint32_t dest;  //rob_id
  uint32_t A;
};

struct RSReturn {
  RSReturn() = default;
};

//  LSB
struct LSBReturn {
  LSBReturn() = default;
  bool pop;
  bool is_load;
  uint32_t rob_id;
  uint32_t data;

  bool add;
  uint32_t add_id;
};

//RF
struct RFReturn {
  RFReturn() = default;
  uint32_t reg[32];
};
#endif //RETURNLIB_H
