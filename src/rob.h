#ifndef ROB_H
#define ROB_H

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
};

template<uint32_t Num>
class ReorderBuffer {
public:
  ReorderBuffer() = default;

  uint32_t push(DecodedIns &decoded_ins) {
    uint32_t t = rear;
    rob[rear].busy = false;
    rob[rear].instruction = decoded_ins;
    rob[rear].state = RoBEntry::Issued;
    rob[rear].dest = decoded_ins.rd;
    rob[rear].value = 0;
    rear = (rear + 1) % Num;
    return t;
  }

  void pop() {
    for (;head != rear; head = (head + 1) % Num) {
      if (rob[head].state != RoBEntry::Committed) {
        return;
      }
    }
  }

private:
  struct RoBEntry {
    enum State {
      Issued,
      Executing,
      Writeback,
      Committed,
    };

    bool busy;
    DecodedIns instruction;
    State state;
    uint32_t dest;
    uint32_t value;
  };

  RoBEntry rob[Num];
  uint32_t head;
  uint32_t rear;
};
#endif //ROB_H
