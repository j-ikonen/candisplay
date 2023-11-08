#ifndef FORMAT_DATA_H
#define FORMAT_DATA_H

#include <cstdint>
#include <string>
#include <vector>
#include <unordered_map>
#include <map>


#include "data_types.h"

// enum Transform {
//   kDivide,
//   kMultiply,
//   kAdd,
//   kState,
//   kBitFlag,
//   kNone,
//   kNotDefined
// };

///
// Data for value formatting from CAN payload.
// Load info from json file.
// Always defined:
//   value, name, is_signed, t_type
// Depending on t_type:
//   kState:
//     NOT DEFINED t_value, unit
//   kBitFlag | kNone:
//     NOT DEFINED t_value
struct DataItem {
  bool is_signed;
  uint16_t value;
  std::string name;
  Transform t_type;
  std::string unit;
  double t_value;
};

struct DataFrame {
  DataItem item[4];
};

typedef std::unordered_map<std::string, std::unordered_map<int, std::string>> MapStateData;
typedef std::map<uint32_t, DataFrame> MapValueData;


class FormatData {
 public:
  FormatData();
  ~FormatData();

  int ReadDataFile(std::string filename);

  void GetFilterIds(std::vector<int> &filter_ids);
  void UpdateFrame(CanFrame &fr);
  void PrintValues();

 private:
  MapStateData state_data_;
  MapValueData values_;

};


#endif // FORMAT_DATA_H