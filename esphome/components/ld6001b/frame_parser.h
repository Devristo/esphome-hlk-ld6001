#pragma once

#include <algorithm>
#include <array>
#include <string>
#include <vector>
#include <stdint.h>
#include <cstddef>
#include <cstring>
#include "esphome/components/json/json_util.h"
#include "esphome/core/log.h"

namespace esphome::ld6001b {

enum class MatchResult { INVALID, PARTIAL, COMPLETE };

union FloatBytes {
  float f;
  std::array<uint8_t, 4> bytes;
};

union Uint32Bytes {
  uint32_t u;
  std::array<uint8_t, 4> bytes;
};

struct Person {
  uint32_t id;
  float x;
  float y;
  float z;
  float breath;
  float heartbeat;
};

struct door_window {
  uint8_t x1;
  uint8_t y1;
  uint8_t x2;
  uint8_t y2;
};

struct HeartBeat {
  uint16_t method;
  uint8_t interferes;
  uint32_t scan_interval;
  uint16_t monitor_interval;
  uint16_t heart_beat_interval;
  uint16_t range;
  uint8_t sensitivity;
  door_window door_windows[6];
};

struct ReadParamsResponse {
  std::string softwareVersion;
  float range_res;
  float vel_res;
  int time;
  int prog;
  int range;
  int range_sensitivity;
  int sensitivity;
  int heart_beat_interval;
  int protocol_mode;
  int detection_height;
  int x_nega;
  int x_posi;
  int y_nega;
  int y_posi;
  int monitor_time;
};

enum class ParseState { IDLE, READING_HEADER, VALIDATING, COMPLETE, INVALID };

class FrameHandler {
 public:
  virtual void on_ack_response() {};
  virtual void on_save_param_failed() {};
  virtual void on_read_params_response(const ReadParamsResponse response) {};
  virtual void on_hear_beat_response(const HeartBeat heartbeat) {};
  virtual void on_simple_radar_response(const std::vector<Person> people) {};
  virtual void on_invalid_frame() {};
  virtual ~FrameHandler() = default;
};

class FrameParser {
 public:
  FrameParser(FrameHandler &handler) : state_(ParseState::IDLE), frame_handler_(handler) {}
  ParseState state_;

  // Call this method to push data into the parser
  void push_data(const uint8_t byte) {

    // ESP_LOGD("ld6001b", "push byte %X to %s", byte, format_hex(buffer_).c_str());

    buffer_.push_back(byte);  // Add byte to the buffer
    try_parse_frame_();       // Try to parse frame after every new byte
  }

  template<size_t N> void push_data(const std::array<uint8_t, N> &data) {
    for (const auto &byte : data) {
      push_data(byte);
    }
  }

  void push_float(float value) {
    FloatBytes fb = {value};
    push_data(fb.bytes);
  }

  void push_uint32(uint32_t value) {
    Uint32Bytes u32 = {value};
    push_data(u32.bytes);
  }

 private:
  std::vector<uint8_t> buffer_;         // Input buffer for incoming bytes
  std::vector<uint8_t> current_frame_;  // The current complete frame
  std::size_t body_len_ = 0;            // Length of the body for frames that include it
  FrameHandler &frame_handler_;         // Reference to the frame handler
  bool new_version = false;

  float read_float(const uint8_t *ptr) {
    float value;
    std::memcpy(&value, ptr, sizeof(float));
    return value;
  }

  uint16_t read_uint16(const uint8_t *ptr) {
    uint16_t value;
    std::memcpy(&value, ptr, sizeof(uint16_t));
    return value;
  }

  uint32_t read_uint32(const uint8_t *ptr) {
    uint32_t value;
    std::memcpy(&value, ptr, sizeof(uint32_t));
    return value;
  }

  void replaceAll(std::string& str, const std::string& from, const std::string& to) {
    if (from.empty()) return;
    size_t startPos = 0;
    while ((startPos = str.find(from, startPos)) != std::string::npos) {
        str.replace(startPos, from.length(), to);
        startPos += to.length(); // Move past the replacement
    }
  }

  void try_parse_frame_() {
    bool invalid = false;
    bool partial = false;
    bool complete = false;
    using MatcherFn = MatchResult (FrameParser::*)();

    MatcherFn matchers[] = {
      &FrameParser::match_at_ok_,
      &FrameParser::match_binary_type1_,
      &FrameParser::match_binary_type2_,
      &FrameParser::match_read_response_,
    };

    do {
      invalid = true;
      complete = false;
      partial = false;

      for (const auto &matcher : matchers) {
        MatchResult result = (this->*matcher)();
        if (result == MatchResult::COMPLETE) {
          complete = true;
          invalid = false;
          partial = false;
          break;
        } else if (result == MatchResult::PARTIAL) {
          partial = true;
          invalid = false;
        } else if (result == MatchResult::INVALID) {
          invalid = !partial && true;
        }
      }

      if (complete) {
        state_ = ParseState::IDLE;
      } if (partial) {
        this->state_ = ParseState::READING_HEADER;
      } else if (invalid) {
        state_ = ParseState::INVALID;
        drain_one_byte();
      }

      // invalid buffer
      if (buffer_.size() > 1024) {
          buffer_.clear();
      }
    } while ((buffer_.size() > 0) && invalid);
  }

  MatchResult match_save_para_fail_() {
    std::string marker = "Save Para Fail\r\n"; // 16 chars
    auto buffer_size = buffer_.size();
    auto match_size = std::min<size_t>(16, buffer_size);

    auto match = std::equal(marker.begin(), marker.begin() + match_size, buffer_.begin());
    //ESP_LOGE("ld6001b", "match_save_para_fail_ %x", match);
    if (!match) {
      return MatchResult::INVALID;
    } else if (buffer_size < 16) {
      return MatchResult::PARTIAL;
    } else {
      buffer_.erase(buffer_.begin(), buffer_.begin() + 16);  // Remove the processed part from the buffer
      this->frame_handler_.on_save_param_failed();

      return MatchResult::COMPLETE;
    }
  }
/*
{
"SoftVerison":"1.2D",
"RangeRes":0.055664,
"VelRes":0.096055,
"TIME":100,
"PROG":2,
"Range":300,
"BautRate":115200,
"Sen":2,
"Heart_Time":60,
"Monitor_Time":1
}
*/
  MatchResult match_read_response_() {
    if (buffer_[0] != '{') {
      return MatchResult::INVALID;
    }

    std::string monitor_time_marker = "Monitor_Time";
    auto monitor_time_pos = std::search(buffer_.begin(), buffer_.end(), monitor_time_marker.begin(), monitor_time_marker.end());

    if (monitor_time_pos == buffer_.end()) {
      return MatchResult::PARTIAL;
    }

    std::string end_marker = "}";
    auto end_pos = std::search(monitor_time_pos, buffer_.end(), end_marker.begin(), end_marker.end());

    if (end_pos == buffer_.end()) {
      return MatchResult::PARTIAL;
    }

    std::string new_version_marker = "SoftVerison"; // 11 chars
    auto version_pos = std::search(buffer_.begin(), buffer_.end(), new_version_marker.begin(), new_version_marker.end());
    if (version_pos != buffer_.end()) {
      this->new_version = true;
    }

    std::string response(buffer_.begin(), end_pos);

    ESP_LOGE("ld6001b", "Found READ response: %s", response.c_str());

    buffer_.erase(buffer_.begin(), end_pos + 1);  // Remove the processed part from the buffer

    replaceAll(response, "\x09\x0a", "\r\n");
    replaceAll(response, "\xa3\xba", " ");

    response += "}";

    ESP_LOGW("ld6001b", "Fixed READ response: %s", response.c_str());

    json::parse_json(response, [this](ArduinoJson::JsonObject obj) -> bool {
      ReadParamsResponse read_params_response;
      if (this->new_version)
        read_params_response.softwareVersion = obj["PeopleCntSoftVerison"].as<std::string>();
      else
        read_params_response.softwareVersion = obj["SoftwareVersion"].as<std::string>();
      read_params_response.range_res = obj["RangeRes"].as<float>();
      read_params_response.vel_res = obj["VelRes"].as<float>();
      if (this->new_version) {
        read_params_response.time = obj["TIME"].as<int>();
        read_params_response.prog = obj["PROG"].as<int>();
      } else {
        read_params_response.time = obj["Time"].as<int>();
        read_params_response.prog = obj["Prog"].as<int>();
      }
      read_params_response.range = obj["Range"].as<int>();
      read_params_response.sensitivity = obj["Sen"].as<int>();
      read_params_response.heart_beat_interval = obj["Heart_Time"].as<int>();
      read_params_response.monitor_time = obj["Monitor_Time"].as<float>();

      this->frame_handler_.on_ack_response();
      this->frame_handler_.on_read_params_response(read_params_response);

      return true;
    });


    return MatchResult::COMPLETE;
  }

  MatchResult match_at_ok_() {
    auto buffer_size = buffer_.size();
    auto match_size = std::min<size_t>(3, buffer_size);
    const std::string prefix = "AT+";
    const std::string suffix = "\r\n";

    bool match = std::equal(prefix.begin(), prefix.begin() + match_size, buffer_.begin());

    if (!match) {
      return MatchResult::INVALID;
    } else if (buffer_size < prefix.size()) {
      return MatchResult::PARTIAL;
    }

    size_t pos = prefix.size();

    // Optional '=' and value (anything but '\r')
    if (buffer_size > pos) {
      ++pos;
      while (pos < buffer_size && buffer_[pos] != '\r') {
        ++pos;
      }
    }

    // Now expect "\r\n"
    if (buffer_size < pos + suffix.size()) {
      // Not enough for suffix, check partial
      for (size_t i = 0; i < buffer_size - pos; ++i) {
        if (buffer_[pos + i] != static_cast<uint8_t>(suffix[i]))
          return MatchResult::INVALID;
      }
      return MatchResult::PARTIAL;
    }

    // Check suffix
    if (buffer_[pos] == '\r' && buffer_[pos + 1] == '\n') {

      buffer_.erase(buffer_.begin(), buffer_.begin() + pos+1 + 1);

      this->frame_handler_.on_ack_response();

      return MatchResult::COMPLETE;
    }

    return MatchResult::INVALID;
  }

  MatchResult match_binary_type1_() {
    auto buffer_size = buffer_.size();
    auto match_size = std::min<size_t>(2, buffer_size);
    static const std::array<uint8_t, 2> header = {0x55, 0xAA};

    auto match = std::equal(buffer_.begin(), buffer_.begin() + match_size, header.begin());
    if (!match) {
      return MatchResult::INVALID;
    }

    if (buffer_size < header.size()) {
     return MatchResult::PARTIAL;
    }

    state_ = ParseState::READING_HEADER;

    auto frame_type = buffer_[3];
    body_len_ = buffer_[2];  // Body length is the 3rd byte in type 1

     if (buffer_size < body_len_) {
      // Not enough for body
      return MatchResult::PARTIAL;
     }

     if (!validate_frame()) {
      return MatchResult::INVALID;
     }

     process_binary_type1_response(frame_type);
     return MatchResult::COMPLETE;
  }

  MatchResult match_binary_type2_() {
    auto buffer_size = buffer_.size();
    auto match_size = std::min<size_t>(8, buffer_size);
    static const std::array<uint8_t, 8> header = {0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08};

    bool match = std::equal(header.begin(), header.begin() + match_size, buffer_.begin());
    if (!match) {
      return MatchResult::INVALID;
    } else if (buffer_size < header.size() + 4) {
      return MatchResult::PARTIAL;
    }

    auto body_len_ = read_uint32(&buffer_[8]) + 1;

    if (buffer_size < body_len_) {
      // Not enough for body
      return MatchResult::PARTIAL;
    }

    if (!validate_frame()) {
      return MatchResult::INVALID;
    }

     process_binary_type2_response();

     return MatchResult::COMPLETE;
  }

  void drain_one_byte() {
    ESP_LOGE("ld6001b", "drain single byte from: %s", format_hex_pretty(buffer_).c_str());

    if (!buffer_.empty()) {
      buffer_.erase(buffer_.begin());
    }
  }

  bool validate_frame() {
    auto expected_checksum = buffer_[buffer_.size() - 1];

    if (buffer_[0] == 0x55 && buffer_[1] == 0xAA) {
      uint8_t calculated_checksum = 0;
      for (size_t i = 2; i < buffer_.size() - 1; ++i) {
        calculated_checksum ^= buffer_[i];
      }

      return expected_checksum == calculated_checksum;
    } else {
      uint8_t calculted_checksum = 0;
      for (size_t i = 12; i < 16; ++i) {
        calculted_checksum ^= buffer_[i];
      }
      for (size_t i = 32; i < buffer_.size() - 1; ++i) {
        calculted_checksum ^= buffer_[i];
      }

      return expected_checksum == calculted_checksum;
    }
  }

  void process_binary_type1_response(uint8_t type) {
    if (type == 0x01) {
      auto people_count = buffer_[4];  // Assuming 4th byte is people count

      std::vector<Person> people;
      people.reserve(people_count);

      for (size_t i = 0; i < people_count; ++i) {
        auto offset = i * 5 + 6;  // Start reading from the 6rd byte
        Person person = {
            .id = i,
            .x = (float)buffer_[offset + 0],
            .y = (float)buffer_[offset + 1],
            .z = (float)buffer_[offset + 2],
            .breath = (float)buffer_[offset + 3],
            .heartbeat = (float)buffer_[offset + 4],
        };

        people.push_back(person);
      }
      this->frame_handler_.on_simple_radar_response(people);  // Assuming 4th byte is people count
    } else {
      HeartBeat heartbeat;
      heartbeat.method = buffer_[5] << 4 | buffer_[4];
      heartbeat.interferes = buffer_[6];
      heartbeat.scan_interval = read_uint32(&buffer_[6]);
      heartbeat.monitor_interval = read_uint16(&buffer_[10]);
      heartbeat.heart_beat_interval = read_uint16(&buffer_[12]);
      heartbeat.range = read_uint16(&buffer_[14]);
      heartbeat.sensitivity = buffer_[16];
      for (size_t i = 0; i < 6; ++i) {
        auto offset = i * 4 + 17;
        heartbeat.door_windows[i] = {
            .x1 = buffer_[offset + 0],
            .y1 = buffer_[offset + 1],
            .x2 = buffer_[offset + 2],
            .y2 = buffer_[offset + 3],
        };
      }
      this->frame_handler_.on_hear_beat_response(heartbeat);
    }

    buffer_.clear();
  }

  void process_binary_type2_response() {
    buffer_.clear();
  }

};

}  // namespace esphome::ld6001b
