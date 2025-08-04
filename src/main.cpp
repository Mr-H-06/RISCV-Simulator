#include "simulator.h"

int main() {
  std::string filename = "../testcases/naive.data";
  Simulator simulator(filename);
  //Simulator simulator = Simulator();
  simulator.run();
  return 0;
}