#include "simulator.h"

int main() {
  std::string filename = "../testcases/bulgarian.data";
  Simulator simulator(filename);
  //Simulator simulator = Simulator();
  simulator.run();
  return 0;
}