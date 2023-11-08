#include "ui_handler.h"

#include <iostream>
#include <iomanip>

UiHandler::UiHandler() : active_page_(0) {}

UiHandler::~UiHandler() {}

int UiHandler::InitScreen() { return 0; }

template<typename TYPE>
TYPE SetValue(TYPE raw, CanFrame& canfr, UiFrame& uifr) {
  CanItem& ci = uifr.can_item;
  uint8_t idx = uifr.can_item.can_id_idx * 2;
  raw = 0;
  raw |= (canfr.data[idx] << 8); 
  raw |= canfr.data[idx+1]; 

  switch (ci.t_type) {
    case kDivide:   uifr.value = raw / ci.t_value; break;
    case kMultiply: uifr.value = raw * ci.t_value; break;
    case kAdd:      uifr.value = raw + ci.t_value; break;
    default:        uifr.value = raw;
  }


  #ifdef DEBUG
  std::cout << std::hex << std::setw(2) 
            << "id: " << (int)ci.can_id << " idx: " << (int)ci.can_id_idx
            << " name: " << ci.name << " [" << ci.unit << "]\n"
            << "raw value: " << raw
            << " value: " << uifr.value << '\n'
            << std::endl;
  #endif // DEBUG

  return raw;
}


int UiHandler::UpdateValues(std::vector<CanFrame> &can_frame_vec) { 
  for (Page& p : page_vec_) {
    for (UiFrame& uifr : p.frames) {
      for (CanFrame& canfr : can_frame_vec) {
        // Found matching can id in store with received can frame
        if (canfr.can_id == uifr.can_item.can_id) {

          if (uifr.can_item.is_signed) {
            SetValue(uifr.raw.s, canfr, uifr);
          } else {
            SetValue(uifr.raw.u, canfr, uifr);
          } // if signed
        } // if can_id == can_id
      } // for canfr
    } // for uifr
  } // for p
  int ret = can_frame_vec.size();
  can_frame_vec.clear();
  return ret; 
}

const std::vector<uint32_t>& UiHandler::GetFilters() {
  for (Page& p : page_vec_) {
    for (UiFrame& fr : p.frames) {
      // Check for dublicates
      bool is_dublicate = false;
      for (uint32_t id : filters_) {
        if (id == fr.can_item.can_id) {
          is_dublicate = true;
          break;
        }
      }
      if (!is_dublicate) {
        filters_.push_back(fr.can_item.can_id);
      }
    }
  }
  return filters_;
}


void UiHandler::Print() {
// #ifdef DEBUG

  std::string namestr, valuestr;
  int counter = 0;
  int total = 1;
  const int max = page_vec_[active_page_].frames.size();
  for (UiFrame& uifr : page_vec_[active_page_].frames) {
    std::stringstream ssv, ssn;
    CanItem& ci = uifr.can_item;
    std::string nameunit = ci.name;
    nameunit += " [";
    nameunit += ci.unit;
    nameunit += "]";
    ssv << std::setw(15) << std::left << std::fixed << std::setprecision(3) 
        << uifr.value;
    ssn << std::setw(15) << std::left << nameunit;

    namestr += ssn.str();
    valuestr += ssv.str();


    if (counter >= 4 || total >= max) {
      std::cout << namestr << '\n' << valuestr << '\n' << std::endl;
      namestr.clear();
      valuestr.clear();
      counter = 0;
    }
    counter++;
    total++;
  }



// #endif // DEBUG
}
