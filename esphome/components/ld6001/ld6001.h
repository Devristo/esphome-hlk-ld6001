#pragma once

#include <unordered_set>
#include <unordered_map>
#include <iomanip>
#include <map>
#include "esphome/components/uart/uart.h"
#include "esphome/core/component.h"
#include "esphome/core/defines.h"
#include "esphome/core/helpers.h"
#include "esphome/core/preferences.h"
#include "frame_parser.h"

#ifdef USE_SENSOR
#include "esphome/components/sensor/sensor.h"
#endif
#ifdef USE_NUMBER
#include "esphome/components/number/number.h"
#endif
#ifdef USE_SWITCH
#include "esphome/components/switch/switch.h"
#endif
#ifdef USE_BUTTON
#include "esphome/components/button/button.h"
#endif
#ifdef USE_SELECT
#include "esphome/components/select/select.h"
#endif
#ifdef USE_TEXT_SENSOR
#include "esphome/components/text_sensor/text_sensor.h"
#endif
#ifdef USE_BINARY_SENSOR
#include "esphome/components/binary_sensor/binary_sensor.h"
#endif

#ifndef M_PI
#define M_PI 3.14
#endif

namespace esphome {
namespace ld6001 {

// Constants
static const uint8_t DEFAULT_PRESENCE_TIMEOUT = 5;  // Timeout to reset presense status 5 sec.
static const uint16_t MAX_LINE_LENGTH = 1024;          // Max characters for serial buffer
static const uint8_t MAX_ZONES = 4;                 // Max 3 Zones in LD6001

struct TargetInfo {
  uint8_t targets;
  Target target_data[MAX_TARGETS];
};

// Zone coordinate struct
struct Zone {
  int16_t x1 = 0;
  int16_t y1 = 0;
  int16_t x2 = 0;
  int16_t y2 = 0;
  uint8_t target_count = 0;

  bool contains(const int16_t x, const int16_t y) const {
    return (x >= this->x1 && x <= this->x2 && y >= this->y1 && y <= this->y2);
  }
};

#ifdef USE_NUMBER
struct ZoneOfNumbers {
  number::Number *x1 = nullptr;
  number::Number *y1 = nullptr;
  number::Number *x2 = nullptr;
  number::Number *y2 = nullptr;
};
#endif

class LD6001Component : public PollingComponent, public uart::UARTDevice, public FrameHandler {
#ifdef USE_SENSOR
  SUB_SENSOR(target_count)
#endif

#ifdef USE_TEXT_SENSOR
  SUB_TEXT_SENSOR(version)
  SUB_TEXT_SENSOR(mac)
#endif

 public:
  LD6001Component();
  void setup() override;
  void dump_config() override;
  void loop() override;
  void update() override;

  void set_throttle(uint16_t value) { this->throttle_ = value; };

  void on_radar_response(const RadarResponse &response) override;
  void on_status_response(const StatusResponse &response) override;

#ifdef USE_SENSOR
  void set_move_x_sensor(uint8_t target, sensor::Sensor *s);
  void set_move_y_sensor(uint8_t target, sensor::Sensor *s);
  void set_move_pitch_angle_sensor(uint8_t target, sensor::Sensor *s);
  void set_move_horizontal_angle_sensor(uint8_t target, sensor::Sensor *s);
  void set_move_distance_sensor(uint8_t target, sensor::Sensor *s);
  void set_zone_target_count_sensor(uint8_t zone, sensor::Sensor *s);
#endif

#ifdef USE_NUMBER
  void set_zone_coordinate(uint8_t zone);
  void set_zone_numbers(uint8_t zone, number::Number *x1, number::Number *y1, number::Number *x2, number::Number *y2);
#endif

protected:
  void send_version_request_();
  void send_radar_request_();

  void update_sensors_();
  void read_version_frame_(const std::vector<uint8_t> &buffer);
  void read_radar_frame_(const uint8_t *buffer);
  void update_last_seen_(uint8_t target_id);

  TargetInfo target_info_ = {};
  Zone zone_config_[MAX_ZONES];
  uint32_t last_periodic_millis_ = 0;
  uint32_t presence_millis_ = 0;
  uint32_t still_presence_millis_ = 0;
  uint32_t moving_presence_millis_ = 0;
  uint16_t throttle_ = 1000;
  uint16_t timeout_ = 5;

  std::deque<uint8_t> announce_entry_;
  std::deque<uint8_t> removed_targets_;
  std::unordered_map<uint8_t, uint32_t> entry_times_;
  std::unordered_map<uint8_t, uint32_t> last_seen_times_;

  FrameParser frame_iter_;

  uint8_t zone_type_ = 0;
  std::string version_{};
  std::string mac_{};
#ifdef USE_NUMBER
  ESPPreferenceObject pref_;  // only used when numbers are in use
  ZoneOfNumbers zone_numbers_[MAX_ZONES];
#endif
#ifdef USE_SENSOR
  std::vector<sensor::Sensor *> move_x_sensors_ = std::vector<sensor::Sensor *>(MAX_TARGETS);
  std::vector<sensor::Sensor *> move_y_sensors_ = std::vector<sensor::Sensor *>(MAX_TARGETS);
  std::vector<sensor::Sensor *> move_pitch_angle_sensors_ = std::vector<sensor::Sensor *>(MAX_TARGETS);
  std::vector<sensor::Sensor *> move_horizontal_angle_sensors_ = std::vector<sensor::Sensor *>(MAX_TARGETS);
  std::vector<sensor::Sensor *> move_distance_sensors_ = std::vector<sensor::Sensor *>(MAX_TARGETS);
  std::vector<sensor::Sensor *> zone_target_count_sensors_ = std::vector<sensor::Sensor *>(MAX_ZONES);
#endif
};

}  // namespace ld6001
}  // namespace esphome
