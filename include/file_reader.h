#ifndef FILE_READER_H
#define FILE_READER_H

#include <map>
#include <unordered_map>
#include <vector>
#include <string>
#include <cstdint>
#include "data_types.h"

#include <nlohmann/json.hpp>


using json = nlohmann::json;

typedef std::unordered_map<std::string, std::unordered_map<int, std::string>> MapStateData;



class FileReader {
 public:
  FileReader();
  ~FileReader();


  void Init(std::vector<Page> &page_vec, 
            std::string ui_filename = "ui_frames.json");

  void GetScreenResolution(int &x, int &y);
  std::string GetCanIface();

 private:
  json j_setup_;
  std::string can_filename_;
  std::string ui_filename_;

  // data of possible values in CAN datastream 
  // <CanItem.name, CanItem>
  std::map<std::string, CanItem> can_datastream_;
  // Bitmask and state value data
  // <CanItem.name, <UiFrame.value, value string>>
  MapStateData state_data_;
  // Table to match Transform enum to strings
  std::unordered_map<std::string, Transform> tf_table_;

  int ReadUiFile(std::vector<Page> &page_vec);
  int ReadCanFile();
  void SetFrames(std::vector<Page> &page_vec);
};


#endif // FILE_READER_H