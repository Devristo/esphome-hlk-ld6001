#pragma once

#include <algorithm>
#include <array>
#include <string>
#include <vector>
#include <stdint.h>
#include <cstddef>
#include <cstring>
#include "esphome/core/log.h"

namespace esphome {
namespace ld6001 {
static const uint8_t MAX_TARGETS = 10;

struct StatusResponse {
  uint8_t software_version_minor;
  uint8_t software_version_major;

  uint8_t hardware_version_minor;
  uint8_t hardware_version_major;
  bool initialized;

  static StatusResponse create(const std::vector<uint8_t> &buffer) {
    return StatusResponse{.software_version_minor = buffer[4],
                          .software_version_major = buffer[5],
                          .hardware_version_minor = buffer[6],
                          .hardware_version_major = buffer[7],
                          .initialized = buffer[9] == 0x00};
  }
};

struct Target {
  uint8_t id;
  uint8_t pitch_angle;
  uint8_t horizontal_angle;
  uint16_t distance;
  int16_t x;
  int16_t y;
};

struct RadarResponse {
  uint8_t targets;
  uint8_t fault_status;
  Target people[MAX_TARGETS];

  static RadarResponse create(const std::vector<uint8_t> &buffer) {
    RadarResponse response;
    response.fault_status = buffer[4];
    response.targets = buffer[5];

    for (int target = 0; target < MAX_TARGETS; target++) {
      size_t offset = 12 + target * 8;
      uint8_t id = 0;
      uint16_t distance = 0;
      uint8_t pitch_angle = 0;
      uint8_t horizontal_angle = 0;
      int16_t coord_x = 0;
      int16_t coord_y = 0;

      if (target < response.targets) {
        id = buffer[offset];
        distance = buffer[offset + 1] * 10;
        pitch_angle = buffer[offset + 2];
        horizontal_angle = buffer[offset + 3];
        coord_x = static_cast<int8_t>(buffer[offset + 6]) * 10;
        coord_y = static_cast<int8_t>(buffer[offset + 7]) * 10;
      }

      response.people[target] = Target{.id = id,
                                       .pitch_angle = pitch_angle,
                                       .horizontal_angle = horizontal_angle,
                                       .distance = distance,
                                       .x = coord_x,
                                       .y = coord_y};
    }

    return response;
  }
};

class FrameHandler {
 public:
  virtual void on_radar_response(const RadarResponse &response) {};
  virtual void on_status_response(const StatusResponse &response) {};
  virtual ~FrameHandler() = default;
};

class FrameParser {
 public:
  FrameParser(FrameHandler &handler) : handler_(&handler) {
    current_.reserve(256);
    buffer_.reserve(256);
  };

  void push_data(unsigned char byte) {
    buffer_.push_back(byte);  // Add byte to the buffer
    try_parse_frame_();       // Try to parse frame after every new byte
  };

  static uint8_t get_iterator_checksum(std::vector<uint8_t>::iterator begin, std::vector<uint8_t>::iterator end) {
    uint8_t sum = 0;
  
    for (auto it = begin; it != end; it++) {
      sum += *it;
    }
  
    return sum;
  };

 protected:
  void drain_until_frame_start_() {
    while (!buffer_.empty() && buffer_[0] != FRAME_START) {
      buffer_.erase(buffer_.begin());
    }
  };
  
  void process_frame_(const std::vector<uint8_t> &frame) {
    uint8_t msg_type = frame[1];
  
    switch (msg_type) {
      case 0x11:
        this->handler_->on_status_response(StatusResponse::create(frame));
        break;
      case 0x62:
        this->handler_->on_radar_response(RadarResponse::create(frame));
        break;
      default:
        // ESP_LOGW("ld6001", "Unknown message type: 0x%02X", msg_type);
        break;
    }
  };

  void try_parse_frame_() {
    // Skip until we have a frame start byte
    drain_until_frame_start_();
  
    // As long as we have a buffer of at least the header size, we can try to process it.
    while (buffer_.size() >= HEADER_SIZE) {
      uint8_t body_len = buffer_[2];
      size_t total_len = HEADER_SIZE + body_len;
  
      if (buffer_.size() < total_len)
        break;
  
      if (buffer_[total_len - 1] != FRAME_END) {
        drain_until_frame_start_();
        continue;
      }
  
      auto checksum = buffer_[total_len - 2];
      auto expected_checksum = get_iterator_checksum(buffer_.begin(), buffer_.begin() + total_len - 2);
  
      if (checksum != expected_checksum) {
        ESP_LOGW("ld6001", "Checksum mismatch: expected %02X, got %02X", expected_checksum, checksum);
        ESP_LOGW("ld6001", "%s", format_hex_pretty(buffer_).c_str());
  
        buffer_.erase(buffer_.begin());
        continue;
      }
  
      current_.assign(buffer_.begin(), buffer_.begin() + total_len);
      buffer_.erase(buffer_.begin(), buffer_.begin() + total_len);
  
      this->process_frame_(current_);
    }
  };

 private:
  std::vector<uint8_t> buffer_;
  std::vector<uint8_t> current_;
  FrameHandler *handler_;

  static constexpr uint8_t FRAME_START = 0x4D;
  static constexpr uint8_t FRAME_END = 0x4A;
  static constexpr size_t HEADER_SIZE = 6;
};

}  // namespace ld6001
}  // namespace esphome