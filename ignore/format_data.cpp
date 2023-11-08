#include "format_data.h"

#include <iomanip>
#include <iostream>
#include <fstream>
#include <map>

#include <nlohmann/json.hpp>

using json = nlohmann::json;



FormatData::FormatData() {}

FormatData::~FormatData() {}


int FormatData::ReadDataFile(std::string filename) {
  std::ifstream in;
  in.exceptions(std::ifstream::failbit | std::ifstream::badbit);
  json j;
  try {
    in.open(filename);
    in >> j;
    in.close();
  } catch (std::ifstream::failure e) {
    std::cerr << "Exception opening/reading/closing file\n";
    return -1;
  }

  const std::string items[4] = {"item_0", "item_1", "item_2", "item_3"};
  std::map<std::string, Transform> t_map;
  t_map.insert(std::pair<std::string, Transform>("divide", kDivide));
  t_map.insert(std::pair<std::string, Transform>("multiply", kMultiply));
  t_map.insert(std::pair<std::string, Transform>("add", kAdd));
  t_map.insert(std::pair<std::string, Transform>("state", kState));
  t_map.insert(std::pair<std::string, Transform>("bit_flag", kBitFlag));
  t_map.insert(std::pair<std::string, Transform>("none", kNone));

  try {
    // Frame x iterate
    for (json::iterator frame_it = j.begin(); frame_it != j.end(); ++frame_it) {
      std::string key_str = frame_it.key();
      // if Frame x
      if (key_str.compare(0, 5, "Frame") == 0) {
        json json_fr = frame_it.value();
        DataFrame df;
        uint32_t can_id = json_fr.at("can_id").get<uint32_t>();

        // item_x
        for (int i = 0; i < 4; ++i) {
          json j_item = json_fr.at(items[i]);
          std::string t_type_str  = j_item.at("transform_type").get<std::string>();
          df.item[i].name         = j_item.at("name").get<std::string>();
          df.item[i].is_signed    = j_item.at("is_signed").get<bool>();
          df.item[i].t_type       = t_map[t_type_str];

          if (df.item[i].t_type < kState) {
            df.item[i].unit = j_item.at("unit").get<std::string>();
            df.item[i].t_value = j_item.at("transform_value").get<double>();
          } else if (df.item[i].t_type == kBitFlag || df.item[i].t_type == kNone) {
            df.item[i].unit = j_item.at("unit").get<std::string>();
          }
        }
        values_.emplace(can_id, df);

      // bit or state data instread of Frame x
      } else if (key_str.compare(0, 10, "state_data") == 0 || 
                 key_str.compare(0, 13, "bit_flag_data") == 0) {
        json json_data = frame_it.value();

        // "name" for value
        for (auto &jo : json_data.items()) {
          std::string name_key = jo.key();
          std::unordered_map<int, std::string> value_map;

          // States or bitmasks for "name"
          for (auto &ja : jo.value().items()) {
            json j2map = ja.value();

            int state_key = j2map.at("value").get<int>();
            std::string state_value = j2map.at("state").get<std::string>();

            value_map.emplace(state_key, state_value);
          }
          state_data_.emplace(name_key, value_map);
        }
      }
    }

  } catch (json::exception &e) {
    std::cout << "FormatData::FormatData: " << e.what() << '\n' 
              << "exception id:" << e.id << '\n' 
              << "Likely syntax error or missing data in can_frames.json" 
              << std::endl;
    return -2;
  }
  return 0;
}

void FormatData::GetFilterIds(std::vector<int> &filter_ids) {
  filter_ids.clear();
  for (auto &x : values_) {
    filter_ids.push_back(x.first);
  }
}


void FormatData::UpdateFrame(CanFrame &fr) {
  DataFrame &df = values_[fr.can_id];
  int dlc = fr.can_dlc;
  for (int i = 0; i < 4; ++i) {
    df.item[i].value = 0;
    for (int j = 0; j < 2; ++j) {
      if (dlc) {
        dlc--;
        df.item[i].value |= (fr.data[i * 2 + j] << ((1-j) * 8));
      } 
    }
    // std::cout << "UpdateFrame: " 
    //           << std::hex 
    //           << std::setw(2) 
    //           << std::setfill('0')
    //           << std::setiosflags(std::ios::uppercase)  
    //           << (int)df.item[i].value << std::endl;
  }
}