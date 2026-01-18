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

namespace esphome::ld6001a {

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
  float vx;
  float vy;
  float vz;
};

struct ReadParamsResponse {
  std::string softwareVersion;
  float range_res;
  float vel_res;
  int time;
  int prog;
  int range;
  int range_sensitivity;
  int heart_beat_interval;
  int protocol_mode;
  int detection_height;
  int x_nega;
  int x_posi;
  int y_nega;
  int y_posi;
  float moving_target_disappearance_time;
  float static_target_disappearance_time;
  float target_exit_time;
};

enum class ParseState { IDLE, READING_HEADER, VALIDATING, COMPLETE, INVALID };

class FrameHandler {
 public:
  virtual void on_ack_response() {};
  virtual void on_save_param_failed() {};
  virtual void on_read_params_response(const ReadParamsResponse response){};
  virtual void on_simple_radar_response(const uint8_t people_counted) {};
  virtual void on_detailed_radar_response(const std::vector<Person> people) {};
  virtual void on_invalid_frame() {};
  virtual ~FrameHandler() = default;
};

class FrameParser {
 public:
  FrameParser(FrameHandler &handler) : state_(ParseState::IDLE), frame_handler_(handler) {}
  ParseState state_;

  // Call this method to push data into the parser
  void push_data(const uint8_t byte) {

    // ESP_LOGD("ld6001a", "push byte %X to %s", byte, format_hex(buffer_).c_str());

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

  MatchResult match_read_response_() {
    if (buffer_[0] != '{') {
      return MatchResult::INVALID;
    }

    std::string target_exit_marker = "Target exit";
    auto target_exit_pos = std::search(buffer_.begin(), buffer_.end(), target_exit_marker.begin(), target_exit_marker.end());

    if (target_exit_pos == buffer_.end()) {
      return MatchResult::PARTIAL;
    }

    std::string end_marker = "}";
    auto end_pos = std::search(target_exit_pos, buffer_.end(), end_marker.begin(), end_marker.end());

    // Some older firmware do not return a } as end token.
    if (end_pos == buffer_.end()) {
      std::string new_marker = "s,";
      end_pos = std::search(target_exit_pos, buffer_.end(), new_marker.begin(), new_marker.end());
    }

    // Neither token is found (yet).
    if (end_pos == buffer_.end()) {
      return MatchResult::PARTIAL;
    }

    std::string response(buffer_.begin(), end_pos);

    ESP_LOGD("ld6001a", "Found READ response: %s", response.c_str());

    buffer_.erase(buffer_.begin(), end_pos + 1);  // Remove the processed part from the buffer
  
    // It looks like JSON, but it isnt. Try to fix it
    replaceAll(response, "\x09\x0a", "\r\n");
    replaceAll(response, "\xa3\xba", " ");
    replaceAll(response, "\nMoving target", "\n\"Moving target\":");
    replaceAll(response, "\nStatic target", "\n\"Static target\":");
    replaceAll(response, "\nTarget exit", "\n\"Target exit\":");
    replaceAll(response, "NOP_1.07-01", "\"NOP_1.07-01\"");
    replaceAll(response, "s,", ",");

    response += "}";

    replaceAll(response, ",\r\n}", "\r\n}");

    ESP_LOGW("ld6001a", "Fixed READ response: %s", response.c_str());

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
      read_params_response.range_sensitivity = obj["Sen"].as<int>();
      read_params_response.heart_beat_interval = obj["Heart_Time"].as<int>();
      read_params_response.protocol_mode = obj["Debug"].as<int>();
      read_params_response.detection_height = obj["detectionHeight"].as<int>();
      read_params_response.x_nega = obj["XboundaryN"].as<int>();
      read_params_response.x_posi = obj["XboundaryP"].as<int>();
      read_params_response.y_nega = obj["YboundaryN"].as<int>();
      read_params_response.y_posi = obj["YboundaryP"].as<int>();
      read_params_response.moving_target_disappearance_time = obj["Moving target"].as<float>();
      read_params_response.static_target_disappearance_time = obj["Static target"].as<float>();
      read_params_response.target_exit_time = obj["Target exit"].as<float>();

      this->frame_handler_.on_ack_response();
      this->frame_handler_.on_read_params_response(read_params_response);

      return true;
      // ESP_LOGW("ld6001a", "Parsed READ response: %s", format_hex_pretty(buffer_).c_str());
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
    
    auto frame_type = buffer_[2];
    body_len_ = buffer_[2];  // Body length is the 3rd byte in type 1

     if (buffer_size < body_len_) {
      // Not enough for body
      return MatchResult::PARTIAL;
     }

     if (!validate_frame()) {
      return MatchResult::INVALID;
     }

     buffer_.erase(buffer_.begin(), buffer_.begin() + body_len_);  // Remove the processed part from the buffer
     this->frame_handler_.on_simple_radar_response(buffer_[8]);
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
    ESP_LOGE("ld6001a", "drain single byte from: %s", format_hex_pretty(buffer_).c_str());

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

  void process_binary_type2_response() {
    Uint32Bytes u32 = {.bytes{buffer_[28], buffer_[29], buffer_[30], buffer_[31]}};
    auto people_count = u32.u / 32;  // Assuming 4th byte is people count

    std::vector<Person> people;
    people.reserve(u32.u / 32);

    for (size_t i = 0; i < people_count; ++i) {
      auto offset = i * 32 + 32;  // Start reading from the 33rd byte
      Person person = {
          .id = read_uint32(&buffer_[offset + 4]),
          .x = read_float(&buffer_[offset + 8]),
          .y = read_float(&buffer_[offset + 12]),
          .z = read_float(&buffer_[offset + 16]),
          .vx = read_float(&buffer_[offset + 20]),
          .vy = read_float(&buffer_[offset + 24]),
          .vz = read_float(&buffer_[offset + 28]),
      };

      people.push_back(person);
    }

    buffer_.clear();

    this->frame_handler_.on_detailed_radar_response(people);  // Assuming 4th byte is people count
  }
};

}  // namespace esphome::ld6001a
