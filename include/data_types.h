#ifndef DATA_TYPES_H
#define DATA_TYPES_H

#include <cstdint>
#include <string>
#include <vector>

enum Transform {
  kDivide,
  kMultiply,
  kAdd,
  kState,
  kBitFlag,
  kNone,
  kNotDefined
};

struct Rec {
  int x, y, h, w;
};

struct CanItem {
  uint32_t can_id;
  uint8_t can_id_idx;
  std::string name;
  std::string unit;
  bool is_signed;
  Transform t_type;
  double t_value;
};

struct UiFrame {
  Rec rec;
  union {
    uint16_t u;
    int16_t s;
  } raw;
  double value;
  int precision;
  int flags;
  CanItem can_item;
};

///
// uint32_t can_id: bits 0-28: CAN id, bits 29-31: ERR/RTR/EFF flags
// uint8_t can_dlc: data length code [0,8]
// uint8_t data[8]: data
struct CanFrame {
  uint32_t can_id;
  uint8_t can_dlc;
  uint8_t data[8];
};
// typedef struct can_frame CanFrame;

struct Page {
  std::string name;
  std::vector<UiFrame> frames;
};


#endif // DATA_TYPES_H