#include "ui_data.h"

#include <fstream>
#include <iostream>

#include <nlohmann/json.hpp>


UiData::UiData() {
  using json = nlohmann::json;

  std::ifstream in("ui_frames.json", std::ifstream::in);
  json j;
  in >> j;
  in.close();

  try {
    // Iterate page data and setup objects
    for (json::iterator frame_it = j.begin(); frame_it != j.end(); ++frame_it) {
      // Setup
      if (frame_it.key().compare(0, 5, "setup") == 0) {
        //  TODO: Store setup data

      // Pages
      } else {
        Page page;
        page.name = frame_it.key();
        json j_page = frame_it.value();

        for (auto &j_frame : j_page) {
          UiFrame fr;
          fr.name  = j_frame.at("name").get<std::string>();
          fr.flags = j_frame.at("flags").get<int>();
          fr.rec.x = j_frame.at("x").get<int>();
          fr.rec.y = j_frame.at("y").get<int>();
          fr.rec.h = j_frame.at("h").get<int>();
          fr.rec.w = j_frame.at("w").get<int>();
          page.frame_vec.push_back(fr);
        } // for j_page
        page_vec_.push_back(page);
      } // else
    } // for j
    for (auto &p : page_vec_) {
      std::cout << p.name << ": " << p.frame_vec[1].name 
                << " : " << p.frame_vec[1].rec.y << std::endl;
    }

  } catch (json::exception &e) {
    std::cout << "UiData::UiData: " << e.what() << '\n' 
              << "exception id:" << e.id << '\n' 
              << "Likely syntax error or missing data in can_frames.json" 
              << std::endl;
  } // catch
} // UiData

UiData::~UiData() {}

