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
#include "command_queue.h"
#include "target_tracker.h"
#include "esphome/core/application.h"

#ifdef USE_SENSOR
#include "esphome/components/sensor/sensor.h"
#endif

#ifdef USE_NUMBER
#include "esphome/components/number/number.h"
#endif

#ifdef USE_BUTTON
#include "esphome/components/button/button.h"
#endif

#define MAX_TARGETS 10

namespace esphome {
namespace ld6001b {

static const uint8_t MAX_ZONES = 4;

struct ZoneCoordinates {
  int16_t x1 = 0;
  int16_t y1 = 0;
  int16_t x2 = 0;
  int16_t y2 = 0;
};

// Zone coordinate struct
struct Zone: ZoneCoordinates {
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

class LD6001BComponent : public Component, public uart::UARTDevice, public FrameHandler, protected TargetEventHandler<uint32_t> {
#ifdef USE_SENSOR
  SUB_SENSOR(target_count)
#endif

#ifdef USE_NUMBER
  SUB_NUMBER(heartbeat)
  SUB_NUMBER(scan_interval)
  SUB_NUMBER(monitor_interval)
  SUB_NUMBER(installation_height)
  SUB_NUMBER(long_distance_sensitivity)
  SUB_NUMBER(sensitivity)
  SUB_NUMBER(ground_radius)

  SUB_NUMBER(x_min)
  SUB_NUMBER(x_max)
  SUB_NUMBER(y_min)
  SUB_NUMBER(y_max)
#endif

 public:
  LD6001BComponent();
  void setup() override;
  void dump_config() override;
  void loop() override;

  void start();
  void stop();
  void reset();
  void soft_reset();

  const std::vector<Person> &get_targets() const { return this->detailed_people_response_; }

  Trigger<const std::vector<Person> &> *get_update_trigger() { return &this->update_trigger_; }

  void set_protocol_mode(ProtocolMode mode);
  void set_throttle(uint16_t value) { this->throttle_ = value; };
  void set_reset_pin(InternalGPIOPin *reset_pin) { this->reset_pin_ = reset_pin; }

#ifdef USE_NUMBER
  void set_zone_coordinate(uint8_t zone);
  void set_zone_numbers(uint8_t zone, number::Number *x1, number::Number *y1, number::Number *x2, number::Number *y2);
#endif

  void config_factory_settings();
  void config_distance_sensitivity(uint8_t sensitivity);
  void config_sensitivity(uint8_t sensitivity);
  void config_heartbeat_interval(uint16_t interval_s);
  void config_scan_interval(uint16_t interval_s);
  void config_monitor_interval(uint16_t interval_s);
  void config_ground_radius(uint16_t radius_cm);
  void config_vertical_distance(int distance_cm);

  void config_x_min(int x_min);
  void config_x_max(int x_max);
  void config_y_min(int y_min);
  void config_y_max(int y_max);

  void on_ack_response() override;
  void on_read_params_response(const ReadParamsResponse response) override;
  void on_hear_beat_response(const HeartBeat heartbeat) override;
  void on_simple_radar_response(const std::vector<Person> people);
  void on_detailed_radar_response(const std::vector<Person> people);
  void on_invalid_frame() override;

  virtual void on_target_enter(uint32_t target_id) override;
  virtual void on_target_left(uint32_t target_id, uint32_t dwell_time) override;

#ifdef USE_SENSOR
  void set_move_x_sensor(uint8_t target, sensor::Sensor *s);
  void set_move_y_sensor(uint8_t target, sensor::Sensor *s);
  void set_move_z_sensor(uint8_t target, sensor::Sensor *s);
  void set_move_distance_sensor(uint8_t target, sensor::Sensor *s);
  void set_zone_target_count_sensor(uint8_t zone, sensor::Sensor *s);
#endif

#ifdef USE_BUTTON
  SUB_BUTTON(start)
  SUB_BUTTON(stop)
  SUB_BUTTON(hard_reset)
  SUB_BUTTON(soft_reset)
  SUB_BUTTON(factory_reset)
#endif

 protected:
  FrameParser frame_parser_{*this};
  CommandQueue command_queue_{[this](const std::string &cmd) { this->write_str(cmd.c_str()); }};

  std::vector<Person> detailed_people_response_{};
  uint8_t people_counted_ = 0;
  uint16_t throttle_ = 1000;
  uint16_t last_periodic_millis_ = 0;

  void update_sensors_();

  TargetTracker<Person> target_tracker_{*this};
  Trigger<uint32_t> target_enter_trigger_;
  Trigger<uint32_t, uint32_t> target_left_trigger_;
  Trigger<const std::vector<Person> &> update_trigger_;

  InternalGPIOPin *reset_pin_ = nullptr;

  Zone zone_config_[MAX_ZONES];
#ifdef USE_NUMBER
  ESPPreferenceObject pref_;  // only used when numbers are in use
  ZoneOfNumbers zone_numbers_[MAX_ZONES];
#endif

#ifdef USE_SENSOR
  std::vector<sensor::Sensor *> move_x_sensors_ = std::vector<sensor::Sensor *>(MAX_TARGETS);
  std::vector<sensor::Sensor *> move_y_sensors_ = std::vector<sensor::Sensor *>(MAX_TARGETS);
  std::vector<sensor::Sensor *> move_z_sensors_ = std::vector<sensor::Sensor *>(MAX_TARGETS);
  std::vector<sensor::Sensor *> move_distance_sensors_ = std::vector<sensor::Sensor *>(MAX_TARGETS);
  std::vector<sensor::Sensor *> zone_target_count_sensors_ = std::vector<sensor::Sensor *>(MAX_ZONES);
#endif
};

}  // namespace ld6001b
}  // namespace esphome
