#ifndef UI_DATA_H
#define UI_DATA_H


#include <vector>
#include <string>

#include "format_data.h"
#include "data_types.h"



struct UiFrame {
  DataItem data;
  double fvalue;
  int flags;
  Rec rec;
};

struct Page {
  std::string name;
  std::vector<UiFrame> frame_vec;
};


class UiData {
 public:
  UiData();
  ~UiData();

  int UpdateValues();

 private:
  int active_page_;
  std::vector<Page> page_vec_;
};

#endif // UI_DATA_H