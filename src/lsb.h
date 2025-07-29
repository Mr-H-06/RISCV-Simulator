#ifndef LSB_H
#define LSB_H
#include <rob.h>

template<uint32_t Num>
class LoadStoreBuffer {
public:
  LoadStoreBuffer() = default;
  uint32_t push(uint32_t &dest, DecodedIns &decoded_ins) {

  }

private:
  struct LSBEntry {
    bool is_load;
    uint32_t addr;
    uint32_t data;
    uint32_t rob_id;
  };
};
#endif //LSB_H
