#include "simulator.h"

int main() {
  std::string filename = "../testcases/naive.data";
  Simulator simulator(filename);
  simulator.run();
  return 0;
}