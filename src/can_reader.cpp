#include "can_reader.h"

#include <cstdio>
#include <cstring>
#include <cerrno>
#include <chrono>
#include <iostream>
#include <iomanip>

#include <fcntl.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/time.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <net/if.h>

#include "linux/can.h"
#include "linux/can/bcm.h"


struct CanMsg {
  struct bcm_msg_head msg_head;
  struct can_frame frame;  
};



CanReader::CanReader() {
  // InitCan("can0");
}
CanReader::CanReader(const char *iface) {
  Init(iface);
}


CanReader::~CanReader() {
  if (close(s_) < 0) {
    perror("~CanReader : close");
  }
}


int CanReader::Init(const char *iface) {
  struct ifreq ifr;
  struct sockaddr_can addr;

  s_ = socket(PF_CAN, SOCK_DGRAM, CAN_BCM);
  if (s_ < 0) {
    perror("CanReader : socket");
    return -1;  
  }

  strncpy(ifr.ifr_name, iface, IFNAMSIZ);
  if (ioctl(s_, SIOCGIFINDEX, &ifr) < 0) {
    perror("CanReader : ioctl");
    return -2; 
  }

  addr.can_family  = AF_CAN;
  addr.can_ifindex = ifr.ifr_ifindex;
  if (connect(s_, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
    perror("CanReader : connect");
    return -3; 
  }

  // Set receive timeout on socket to stop read() from blocking
  struct timeval tv;
  tv.tv_sec = 0;
  tv.tv_usec = 10000;
  if (setsockopt(s_, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv)) < 0) {
    perror("CanReader : setsockopt");
    return -4;
  }


  return 0;
}


int CanReader::RxFilterSet() {
  struct CanMsg msg;
  msg.msg_head.opcode  = RX_SETUP;
  msg.msg_head.flags   = 0;
  msg.msg_head.nframes = 0;

  for (auto &id : can_id_filter_) {
    int id_stripped = id & kExtendedFrameFormatMask;
    if (id_stripped > kStandardFrameFormatMask) id |= kExtendedFrameFormat; 
    msg.msg_head.can_id = id;

#ifdef DEBUG
    std::cout << "Setting filter id: " << std::hex << id_stripped; 
    if (id & ~kStandardFrameFormatMask) {
      if (id & kErrorFrame)                std::cout << " - Error Frame";
      if (id & kRemoteTransmissionRequest) std::cout << " - Remote Transmission Request";
      if (id & kExtendedFrameFormat)       std::cout << " - Extended Frame Format";
    } 
    std::cout << std::endl;
#endif

    if (write(s_, &msg, sizeof(msg)) < 0) {
      perror("CanReader : write: RX_SETUP");
      return -1;
    }
  }
  return can_id_filter_.size();
}

int CanReader::SetFilters(const std::vector<uint32_t> &filter_vec) {
  for (uint32_t id : filter_vec) {
    AddFilterId(id);
  }
  return RxFilterSet();
}

void CanReader::AddFilterId(int id) { can_id_filter_.push_back(id); }


void CanReader::CanRead() {
  struct CanMsg msg;
  const int kDelayMs = 10;

  // Run until CanReader::EndThread() called
  while (thread_run_) {
    ssize_t nbytes;

    // Block until timeout
    nbytes = read(s_, &msg, sizeof(msg));
    if (nbytes < 0) {
      if (errno == EAGAIN) {  // socket read timeout
        continue;
      } else {
        perror("CanReader : read");
        std::this_thread::sleep_for(std::chrono::milliseconds(kDelayMs)); 
      }

    } else if (nbytes < (ssize_t)sizeof(msg)) {
      std::cerr << "CanReader : read: incomplete BCM message." << std::endl;
      std::this_thread::sleep_for(std::chrono::milliseconds(kDelayMs));

    } else {
      CanFrame temp_frame {msg.frame.can_id, msg.frame.can_dlc, {}};
      for (int i = 0; i < temp_frame.can_dlc; ++i) {
        temp_frame.data[i] = msg.frame.data[i];
      }

      std::lock_guard<std::mutex> lock(recv_buffer_mutex_);
      recv_buffer_.push_back(temp_frame);

    }
  }
  // std::cout << "Exciting CanRead thread." << std::endl;
}


std::thread CanReader::StartReadThread() {
  thread_run_ = true;
  return std::thread( [=] { CanRead(); });
}
void CanReader::EndThread() {
  thread_run_ = false;
}

int CanReader::GetFrameBuffer(std::vector<CanFrame> &buf) {
  if (recv_buffer_.size() == 0) return 0;
  buf.clear();

  std::lock_guard<std::mutex> lock(recv_buffer_mutex_);
  recv_buffer_.swap(buf);
  return buf.size();
}