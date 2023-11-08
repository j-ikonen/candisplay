///
//  CAN-Display  
//    -  Reads data from CAN bus, formats them according to provided
//       json data file and shows the values on display.
//
//  TODO: Add some lib for display (SDL???)
//

#include <csignal>
#include <iostream>
#include <iomanip>
#include <thread>
#include <vector>

#include "data_types.h"
#include "can_reader.h"
#include "ui_handler.h"
#include "file_reader.h"
#include "screen.h"


static sig_atomic_t sigval;

static void OnSig(int sig) {
  sigval = (sig_atomic_t)sig;
}


int main (int argc, char** argv) {

  for (int i = 0; i < argc; ++i) {
    std::cout << argv[i] << std::endl;
  }

  {
    Screen screen;
    screen.Init();
    screen.LoadMedia();
    screen.Render();
  }

  // Signal handling
  sigval = 0;
  if (signal(SIGINT, OnSig)  == SIG_ERR || 
      signal(SIGTERM, OnSig) == SIG_ERR) {
    std::cerr << "Failed to set signal handlers." << std::endl;
    return EXIT_FAILURE;
  }



  FileReader file;
  UiHandler ui;
  CanReader can;

  std::vector<CanFrame> frame_buf;
  std::thread t_can;

  file.Init(ui.PageVecRef());


// REPLACE WITH TRY CATCH
  if (can.Init("vcan0") < 0) {
    std::cerr << "Failed to initialize connection to CAN bus." << std::endl;
    return EXIT_FAILURE;
  }

  if (can.SetFilters(ui.GetFilters()) < 0) return EXIT_FAILURE;

  t_can = can.StartReadThread();

  while (sigval == 0) {
    if (can.GetFrameBuffer(frame_buf)) {
      ui.UpdateValues(frame_buf);
      ui.Print();
    } else {
      std::this_thread::sleep_for(std::chrono::microseconds(100));
    }
  }

  can.EndThread();
  t_can.join();
  std::cout << "\nThread joined. Exiting..." << std::endl;
  return EXIT_SUCCESS;
}


