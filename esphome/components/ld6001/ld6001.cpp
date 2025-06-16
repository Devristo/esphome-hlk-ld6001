#include "ld6001.h"
#include <utility>
#include "esphome/components/mqtt/mqtt_client.h"
#ifdef USE_NUMBER
#include "esphome/components/number/number.h"
#endif
#ifdef USE_SENSOR
#include "esphome/components/sensor/sensor.h"
#endif
#include "esphome/core/component.h"

namespace esphome {
namespace ld6001 {
auto maybe_publish = [](auto *sensor, float new_value) {
  if (sensor == nullptr)
    return;
  float old_value = sensor->state;
  if ((std::isnan(old_value) && !std::isnan(new_value)) || (!std::isnan(old_value) && (old_value != new_value))) {
    sensor->publish_state(new_value);
  }
};

static const char *const TAG = "ld6001";

static const std::array<uint8_t, 6> CMD_GET_VERSION = {0x44, 0x11, 0x00, 0x00, 0x55, 0x4B};
static const std::array<uint8_t, 14> CMD_RADAR_REQUEST_NORMAL = {0x44, 0x62, 0x08, 0x00, 0x10, 0x00, 0x00,
                                                                 0x00, 0x00, 0x00, 0x00, 0x00, 0xBE, 0x4B};
static const std::array<uint8_t, 14> CMD_RADAR_REQUEST_PRECISE = {0x44, 0x62, 0x08, 0x00, 0x20, 0x00, 0x00,
                                                                  0x00, 0x00, 0x00, 0x00, 0x00, 0xCE, 0x4B};

LD6001Component::LD6001Component() : PollingComponent(500), frame_iter_(FrameParser(*this)) {}

void LD6001Component::setup() {
  ESP_LOGCONFIG(TAG, "Setting up HLK-LD6001...");
  this->send_version_request_();
}

void LD6001Component::dump_config() {
  ESP_LOGCONFIG(TAG, "HLK-LD6001 Human motion tracking radar module:");
#ifdef USE_SENSOR
  LOG_SENSOR("  ", "TargetCountSensor", this->target_count_sensor_);
  for (sensor::Sensor *s : this->move_x_sensors_) {
    LOG_SENSOR("  ", "NthTargetXSensor", s);
  }
  for (sensor::Sensor *s : this->move_y_sensors_) {
    LOG_SENSOR("  ", "NthTargetYSensor", s);
  }
  for (sensor::Sensor *s : this->move_pitch_angle_sensors_) {
    LOG_SENSOR("  ", "NthTargetPitchAngleSensor", s);
  }
  for (sensor::Sensor *s : this->move_horizontal_angle_sensors_) {
    LOG_SENSOR("  ", "NthTargetHorizontalAngleSensor", s);
  }
  for (sensor::Sensor *s : this->move_distance_sensors_) {
    LOG_SENSOR("  ", "NthTargetDistanceSensor", s);
  }
  for (sensor::Sensor *s : this->zone_target_count_sensors_) {
    LOG_SENSOR("  ", "NthZoneTargetCountSensor", s);
  }
#endif

  ESP_LOGCONFIG(TAG, "  Throttle : %ums", this->throttle_);
  ESP_LOGCONFIG(TAG, "  MAC Address : %s", const_cast<char *>(this->mac_.c_str()));
  ESP_LOGCONFIG(TAG, "  Firmware version : %s", const_cast<char *>(this->version_.c_str()));
}

void LD6001Component::loop() {
  auto start = millis();

  while (this->available()) {
    uint8_t byte;
    if (this->read_byte(&byte)) {
      this->frame_iter_.push_data(byte);
    }
  }
}

void LD6001Component::on_status_response(const StatusResponse &response) {
  std::string version =
      str_sprintf("HW v%d.%02d / SW v%d.%02d", response.hardware_version_major, response.hardware_version_minor,
                  response.software_version_major, response.software_version_minor);

#ifdef USE_TEXT_SENSOR
  if (this->version_text_sensor_ != nullptr) {
    this->version_text_sensor_->publish_state(version);
  }
#endif
}

void LD6001Component::update() {
  static int counter = 0;
  this->send_radar_request_();

  if ((counter++) % 100 == 0) {
    this->send_version_request_();
  }

  this->update_sensors_();
}

void LD6001Component::update_sensors_() {
#ifdef USE_SENSOR
  /*
     Reduce data update rate to prevent home assistant database size grow fast
  */
  int32_t current_millis = millis();
  if (current_millis - last_periodic_millis_ < this->throttle_) {
    return;
  }

  last_periodic_millis_ = current_millis;

  maybe_publish(this->target_count_sensor_, this->target_info_.targets);

  uint8_t targets = this->target_info_.targets;
  for (size_t i = 0; i < MAX_TARGETS; i++) {
    Target target = this->target_info_.target_data[i];
    auto coord_x = i < targets ? target.x : NAN;
    auto coord_y = i < targets ? target.y : NAN;
    auto distance = i < targets ? target.distance : NAN;
    auto pitch_angle = i < targets ? target.pitch_angle : NAN;
    auto horizontal_angle = i < targets ? target.horizontal_angle : NAN;

    maybe_publish(this->move_x_sensors_[i], coord_x);
    maybe_publish(this->move_y_sensors_[i], coord_y);
    maybe_publish(this->move_distance_sensors_[i], distance);
    maybe_publish(this->move_pitch_angle_sensors_[i], pitch_angle);
    maybe_publish(this->move_horizontal_angle_sensors_[i], horizontal_angle);
  }

  for (size_t index = 0; index < MAX_ZONES; ++index) {
    auto &zone = this->zone_config_[index];
    zone.target_count = 0;

    for (size_t i = 0; i < targets; i++) {
      auto person = this->target_info_.target_data[i];

      auto target_id = person.id;
      auto coord_x = person.x;
      auto coord_y = person.y;

      if (zone.contains(coord_x * 100, coord_y * 100)) {
        zone.target_count++;
      }
    }
    maybe_publish(this->zone_target_count_sensors_[index], zone.target_count);
  }
#endif

  auto people = std::vector<Target>(
    this->target_info_.target_data,
    this->target_info_.target_data + this->target_info_.targets
  );

  this->update_trigger_.trigger(people);
}

void LD6001Component::on_radar_response(const RadarResponse &response) {
  this->target_info_.targets = response.targets;

  memcpy(this->target_info_.target_data, response.people, sizeof(this->target_info_.target_data));
  auto people = std::vector<Target>(response.people, response.people + response.targets);
  this->target_tracker_.update(people);
}

void LD6001Component::on_target_enter(uint8_t target_id) {
  ESP_LOGW(TAG, "Target %d entered view", target_id);
  this->target_enter_trigger_.trigger(target_id);
}

void LD6001Component::on_target_left(uint8_t target_id, uint32_t dwell_time) {
  ESP_LOGW(TAG, "Target %d left view, dwell time: %d seconds", target_id, dwell_time);
  this->target_left_trigger_.trigger(target_id, dwell_time);
}

// Get LD6001 firmware version
void LD6001Component::send_version_request_() {
  ESP_LOGW(TAG, "Sending get version request");
  this->write_array(CMD_GET_VERSION);
}

void LD6001Component::send_radar_request_() {
  ESP_LOGV(TAG, "Sending radar request");
  this->write_array(CMD_RADAR_REQUEST_NORMAL);
}

#ifdef USE_NUMBER
void LD6001Component::set_zone_coordinate(uint8_t zone) {
  number::Number *x1sens = this->zone_numbers_[zone].x1;
  number::Number *y1sens = this->zone_numbers_[zone].y1;
  number::Number *x2sens = this->zone_numbers_[zone].x2;
  number::Number *y2sens = this->zone_numbers_[zone].y2;
  if (!x1sens->has_state() || !y1sens->has_state() || !x2sens->has_state() || !y2sens->has_state()) {
    return;
  }

  ESP_LOGW(TAG, "Set new coordinates for zone %d: (%d, %d) (%d, %d)", zone, static_cast<int>(x1sens->state),
           static_cast<int>(y1sens->state), static_cast<int>(x2sens->state), static_cast<int>(y2sens->state));

  this->zone_config_[zone].x1 = static_cast<int>(x1sens->state);
  this->zone_config_[zone].y1 = static_cast<int>(y1sens->state);
  this->zone_config_[zone].x2 = static_cast<int>(x2sens->state);
  this->zone_config_[zone].y2 = static_cast<int>(y2sens->state);
}

void LD6001Component::set_zone_numbers(uint8_t zone, number::Number *x1, number::Number *y1, number::Number *x2,
                                       number::Number *y2) {
  if (zone < MAX_ZONES) {
    this->zone_numbers_[zone].x1 = x1;
    this->zone_numbers_[zone].y1 = y1;
    this->zone_numbers_[zone].x2 = x2;
    this->zone_numbers_[zone].y2 = y2;
  }
}
#endif

#ifdef USE_SENSOR
void LD6001Component::set_move_x_sensor(uint8_t target, sensor::Sensor *s) { this->move_x_sensors_[target] = s; }
void LD6001Component::set_move_y_sensor(uint8_t target, sensor::Sensor *s) { this->move_y_sensors_[target] = s; }
void LD6001Component::set_move_pitch_angle_sensor(uint8_t target, sensor::Sensor *s) {
  this->move_pitch_angle_sensors_[target] = s;
}
void LD6001Component::set_move_horizontal_angle_sensor(uint8_t target, sensor::Sensor *s) {
  this->move_horizontal_angle_sensors_[target] = s;
}
void LD6001Component::set_move_distance_sensor(uint8_t target, sensor::Sensor *s) {
  this->move_distance_sensors_[target] = s;
}
void LD6001Component::set_zone_target_count_sensor(uint8_t zone, sensor::Sensor *s) {
  this->zone_target_count_sensors_[zone] = s;
}
#endif

}  // namespace ld6001
}  // namespace esphome
