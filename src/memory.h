#ifndef DECODE_H
#define DECODE_H
#include <iostream>
#include <cstdint>
#include <fstream>
#include <unordered_map>

std::unordered_map<char, uint32_t> hex = {
  {'0', 0}, {'1', 1}, {'2', 2}, {'3', 3},
  {'4', 4}, {'5', 5}, {'6', 6}, {'7', 7},
  {'8', 8}, {'9', 9},
  {'A', 10}, {'B', 11}, {'C', 12}, {'D', 13},
  {'E', 14}, {'F', 15},
  {'a', 10}, {'b', 11}, {'c', 12}, {'d', 13},
  {'e', 14}, {'f', 15}
};

class Memory {
public:
  Memory() {
    std::string line;
    uint32_t location;
    while (std::getline(std::cin, line)) {
      if (line[0] == '@') {
        location = 0;
        for (int i = 1; i <= 8; ++i) {
          location <<= 4;
          location += hex[line[i]];
        }
      } else {
        uint32_t num = (line.size() + 1) / 3;
        for (uint32_t i = 0; i < num; ++i) {
          uint32_t code = (hex[line[i * 3]] << 4) + hex[line[i * 3 + 1]];
          ins[location] = code;
          ++location;
        }
      }
    }
  }

  Memory(std::string &filename) {
    std::ifstream file(filename);
    if (!file.is_open()) {
      std::cerr << "Failed to open" << '\n';
      exit(1);
    }
    std::string line;
    uint32_t location;
    while (std::getline(file, line)) {
      if (line[0] == '@') {
        location = 0;
        for (int i = 1; i <= 8; ++i) {
          location <<= 4;
          location += hex[line[i]];
        }
      } else {
        uint32_t num = (line.size() + 1) / 3;
        for (uint32_t i = 0; i < num; ++i) {
          uint32_t code = (hex[line[i * 3]] << 4) + hex[line[i * 3 + 1]];
          ins[location] = code;
          ++location;
        }
      }
    }
  }

  uint32_t getIns(uint32_t location) {
    return ins[location];
  }
private:
  std::unordered_map<uint32_t, uint32_t> ins;
  uint32_t mem[32];
  uint32_t reg[32];
};
#endif //DECODE_H
