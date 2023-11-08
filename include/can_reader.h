#ifndef CAN_READER_H
#define CAN_READER_H

#include <vector>
#include <thread>
#include <mutex>
#include <atomic>

#include "data_types.h"


///
// Class for reading CAN bus data in a thread.
// Ignores Remote transmission request and error frames.
// Initiates CAN with SocketCAN library to BCM mode to avoid filtering with
// software. 
// Add filter ID's with AddFilterId().
// Set filter ID's with RxFilterSet().
// Get CAN frames to vector with GetFrameBuffer(). returns n of frames in buf
class CanReader {
 public:

  CanReader();
  CanReader(const char *iface);
  ~CanReader();
  
  // Return errno on error or 1 on success
  int Init(const char *iface);

  // Send RX_SETUP msg to setup filter for received CAN id's
  // Returns number of id's set or -1 on error
  int RxFilterSet();
  int SetFilters(const std::vector<uint32_t> &filter_vec);
  void AddFilterId(int id);

  // Starts a thread to read CAN frames to FrameBuffer
  // Call EndThread before thread::join() to exit 
  std::thread StartReadThread();
  void EndThread();

  int GetFrameBuffer(std::vector<CanFrame> &buf);

 private:
  const int kErrorFrame                = 0x20000000;
  const int kRemoteTransmissionRequest = 0x40000000;
  const int kExtendedFrameFormat       = 0x80000000;

  const int kStandardFrameFormatMask   = 0x7FF;
  const int kExtendedFrameFormatMask   = 0x1FFFFFFF;

  int s_;
  std::vector<int> can_id_filter_;
  std::vector<CanFrame> recv_buffer_;
  std::mutex recv_buffer_mutex_;
  std::atomic<bool> thread_run_;

  void CanRead();
};

#endif // CAN_READER_H