#ifndef RETURNLIB_H
#define RETURNLIB_H
#include <cstdint>
constexpr uint32_t Num = 5;
//  fetch
enum Opcode : uint8_t {
  LUI, AUIPC, JAL, JALR, BEQ, BNE, BLT, BGE, BLTU, BGEU,
  LB, LH, LW, LBU, LHU, SB, SH, SW, ADDI, SLTI,
  SLTIU, XORI, ORI, ANDI, SLLI, SRLI, SRAI, ADD, SUB, SLL,
  SLT, SLTU, XOR, SRL, SRA, OR, AND,
  INVALID, EXIT
};
enum OpcodeType : uint8_t {
  U, J, I1, I2, B, S, R, EX, INV
};
//  RoB
class DecodedIns {
public:
  DecodedIns() {
    opcode = INVALID;
    opcode_type = INV;
    rs1 = 0;
    rs2 = 0;
    rd = 0;
    imm = 0;
    pc = 0;
  };

  Opcode opcode;
  OpcodeType opcode_type;
  uint8_t rs1;
  uint8_t rs2; // = shamt when opcode = slli, srli, srai
  uint8_t rd;
  uint32_t imm;
  uint32_t pc;
};

struct RoBReturn {
  RoBReturn() {
    pc_jump = false;
    pc = 0;
    exit = false;
    exit_num = 0;
  }
  bool pc_jump;
  uint32_t pc;
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

  bool prepared;
  bool ready;
  DecodedIns instruction;
  State state;
  uint32_t value;
  uint32_t pc;
  uint32_t rs1val;
  uint32_t rs2val;
};

//  ALU
struct ALUReturn {
  ALUReturn() {
    data = 0;
    branch = false;
  }
  uint32_t data;
  bool branch;
  uint32_t pc_to;
};

//  RS
struct RSEntry {
  RSEntry() {
    opcode = INVALID;
    vj = 0;
    vk = 0;
    //qj = -1;
    //qk = -1;
    dest = 0;
    A = 0;
    pc = 0;
  }
  Opcode opcode;
  uint32_t vj;
  uint32_t vk;
  //int32_t qj;
  //int32_t qk;
  uint32_t dest;  //rob_id
  int32_t A;
  uint32_t pc;
};

struct RSReturn {
  RSReturn() {
    pop = false;
    rob_id = 0;
    aluret = ALUReturn();
    add = false;
    add_id = 0;
  }
  bool pop;
  uint32_t rob_id;
  ALUReturn aluret;

  bool add;
  uint32_t add_id;
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
#endif //RETURNLIB_H
