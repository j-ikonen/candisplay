#include "file_reader.h"

#include <fstream>
#include <iostream>
#include <stdexcept>


FileReader::FileReader() {

  tf_table_.emplace("divide", kDivide);
  tf_table_.emplace("multiply", kMultiply);
  tf_table_.emplace("add", kAdd);
  tf_table_.emplace("state", kState);
  tf_table_.emplace("bit_flag", kBitFlag);
  tf_table_.emplace("none", kNone);
  tf_table_.emplace("", kNotDefined);
}


FileReader::~FileReader() {}


void FileReader::Init(std::vector<Page> &page_vec, std::string ui_filename) {
  ui_filename_ = ui_filename;
  if (ReadUiFile(page_vec) < 0) { return; }
  if (ReadCanFile() < 0) { return; }
  SetFrames(page_vec);
}



void FileReader::GetScreenResolution(int &w, int &h) {
  w = j_setup_.at("screen_resolution").value("w", 800);
  h = j_setup_.at("screen_resolution").value("h", 480);
}

std::string FileReader::GetCanIface() {
  return j_setup_.value("can_iface", "vcan0");
}



int FileReader::ReadCanFile() {
  std::ifstream in;
  in.exceptions(std::ifstream::failbit | std::ifstream::badbit);
  json j;
  try {
    in.open(can_filename_);
    in >> j;
    in.close();
  } catch (std::ifstream::failure e) {
    std::cerr << "ReadCanFile: Exception opening/reading/closing file\n";
    return -1;
  } // catch

  try {
    for (auto& jit : j.items()) {
      std::string key = jit.key();
      json j_fr = jit.value();

      // CAN data for each CAN ID
      if (!(key.compare(0, 5, "Frame"))) {
        uint32_t can_id = j_fr.at("can_id");

        for (auto& jit_i : j_fr.items()) {
          std::string itemkey = jit_i.key();
          json j_item = jit_i.value();

          // CAN Item data
          if (!(itemkey.compare(0, 4, "item"))) {
            CanItem ci;
            ci.can_id = can_id;
            ci.can_id_idx = std::stoi(itemkey.substr(5));
            ci.name = j_item.value("name", "NONDEF");
            ci.unit = j_item.value("unit", "");
            ci.is_signed = j_item.value("is_signed", true);
            ci.t_value = j_item.value("transform_value", 0.0);

            std::string typestr= j_item.value("transform_type", "");
            ci.t_type = tf_table_.at(typestr);

            if (ci.name.compare("SPARE") && 
                ci.name.compare("NONDEF")) {
              // std::cout << "ReadCanFile: ci.name: '" << ci.name << "'" 
                        // << " of length: " << ci.name.size() << std::endl;
              can_datastream_.emplace(ci.name, ci);
            }

          } // if item_x
        } // for jit_i

      // Bit or State value data
      } else { 
        for (auto& jit_bs : j_fr.items()) {
          std::string valuename = jit_bs.key();
          std::unordered_map<int, std::string> bsmap;

          for (auto& jit_v : jit_bs.value().items()) {
            bsmap.emplace(jit_v.value().at("value").get<int>(), 
                          jit_v.value().at("state").get<std::string>());
          } // for jit_v
          state_data_.emplace(valuename, bsmap);
        } // for jit_bs
      } // else
    } // for jit


  } catch (json::exception &e) {
    std::cout << "FileReader::ReadCanFile: " << e.what() << '\n' 
              << "exception id:" << e.id << '\n' 
              << "Likely syntax error or missing data in can_frames.json" 
              << std::endl;
    return -2;
  } // catch
  return 0;
}


int FileReader::ReadUiFile(std::vector<Page> &page_vec) {
  std::ifstream in;
  in.exceptions(std::ifstream::failbit | std::ifstream::badbit);
  json j;
  try {
    in.open(ui_filename_);
    in >> j;
    in.close();
  } catch (std::ifstream::failure e) {
    std::cerr << "ReadUiFile: Exception opening/reading/closing file\n";
    return -1;
  } // catch


  try {
    // Get can data filename first for possible threaded json reading
    j_setup_ = j.at("setup");
    can_filename_ = j_setup_.value("can_data_file", "can_frames.json");

    for (auto& jit : j.items()) {
      // Check for everything not setup
      if (jit.key().compare(0, 5, "setup") != 0) {
        Page page;
        page.name = jit.key();

        for (auto& jit_page : jit.value().items()) {
          json j_page = jit_page.value();
          UiFrame fr;

          // Find matching CanItem for valuename and ui frame data
          std::string valuename = j_page.value("name", "NONDEF");
          fr.rec.x = j_page.value("x", 0);
          fr.rec.y = j_page.value("y", 0);
          fr.rec.h = j_page.value("h", 0);
          fr.rec.w = j_page.value("w", 0);
          fr.flags = j_page.value("flags", 0);
          fr.can_item.name = valuename;
          page.frames.push_back(fr);

        } // for jit_page
        if (page.frames.size()) {
          page_vec.push_back(page);
        } else {
          std::cerr << "FileReader::ReadUiFile: Page '"<< page.name
                    <<"' in '" << ui_filename_ 
                    << "' does not contain any valid items."
                    << std::endl;
        }
      } 
    }
  } catch (json::exception &e) {
    std::cout << "UiData::UiData: " << e.what() << '\n' 
              << "exception id:" << e.id << '\n' 
              << "Likely syntax error or missing data in can_frames.json" 
              << std::endl;
    return -2;
  } // catch json::exception

#ifdef DEBUG
  std::cout << "ReadUiFile: can_filename_: " << can_filename_ << std::endl;
  std::cout << "ReadUiFile: page_vec: frame[0](name, y)" << std::endl;
  for (auto& p : page_vec) {
    std::cout << "(" << p.frames[0].can_item.name << ", " 
              << p.frames[0].rec.y << ")" << std::endl;
  }
#endif // DEBUG

  return 0;
}

void FileReader::SetFrames(std::vector<Page> &page_vec) {

  for (auto it_p = page_vec.begin(); it_p != page_vec.end(); ++it_p) {
    std::vector<UiFrame>& uiv = (*it_p).frames;

    for (auto it_fr = uiv.begin(); it_fr != uiv.end(); ++it_fr) {
      std::string valuename = (*it_fr).can_item.name;

      try {
        // Add can item data to page vector
        (*it_fr).can_item = can_datastream_.at(valuename);
#ifdef DEBUG
        std::cout << (*it_p).name << ": " << (*it_fr).can_item.name 
                  << std::endl;
#endif // DEBUG
      } catch (const std::out_of_range& e) {
        std::cerr << "FileReader::SetFrames: Value '" << valuename
                  << "' not defined in '" << can_filename_ << "'\n'"
                  << e.what() << std::endl;

        // Remove unwanted frames
        it_fr = uiv.erase(it_fr);
      } // catch
    } // for fr

    // Remove empty pages
    if (!uiv.size()) {
      it_p = page_vec.erase(it_p);
    }
  } // for (*it_p)

  if (!page_vec.size()) {
    // THROW EXCEPTION NO FRAMES DEFINED
    std::cerr << "No pages defined." << std::endl;
  } 
} // FileReader::SetFrames