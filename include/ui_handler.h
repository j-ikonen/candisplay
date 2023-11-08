#ifndef UI_HANDLER_H
#define UI_HANDLER_H

#include <vector>
#include <cstdint>

#include "data_types.h"

class UiHandler {
 public:
  UiHandler();
  ~UiHandler();
  
  int InitScreen();
  int UpdateValues(std::vector<CanFrame> &can_frame_vec);
  void Print();

  const std::vector<uint32_t>& GetFilters();

  // int SetPages(std::vector<Page> &pages);
  std::vector<Page>& PageVecRef() { return page_vec_; }

 private:
  int active_page_;
  std::vector<Page> page_vec_;
  std::vector<uint32_t> filters_;


};

#endif // UI_HANDLER_H