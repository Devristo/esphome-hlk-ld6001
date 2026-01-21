#include "ld6001b.h"
#include <utility>
#include "esphome/core/component.h"

namespace esphome {
namespace ld6001b {
static const char *const TAG = "ld6001b";

auto maybe_publish = [](auto *sensor, float new_value) {
  if (sensor == nullptr)
    return;
  float old_value = sensor->state;
  if ((std::isnan(old_value) && !std::isnan(new_value)) || (!std::isnan(old_value) && (old_value != new_value))) {
    sensor->publish_state(new_value);
  }
};

LD6001BComponent::LD6001BComponent() : Component() {}

void LD6001BComponent::setup() {
  ESP_LOGCONFIG(TAG, "Setting up HLK-LD6001B...");

  this->command_queue_.enqueue(Command::ResetCommand());
  this->set_protocol_mode(ProtocolMode::PROTOCOL_MODE_SIMPLE);
  this->start();
  this->command_queue_.enqueue(Command::ReadCommand());

  uint32_t hash = fnv1_hash(App.get_friendly_name());
  this->pref_ = global_preferences->make_preference<ZoneCoordinates[MAX_ZONES]>(hash, true);

  ZoneCoordinates zones[MAX_ZONES];

  if (this->pref_.load(&zones)) {
    ESP_LOGW(TAG, "Loaded %d zones from preferences", MAX_ZONES);

    for (size_t i = 0; i < MAX_ZONES; i++) {
      maybe_publish(this->zone_numbers_[i].x1, zones[i].x1);
      maybe_publish(this->zone_numbers_[i].x2, zones[i].x2);
      maybe_publish(this->zone_numbers_[i].y1, zones[i].y1);
      maybe_publish(this->zone_numbers_[i].y2, zones[i].y2);

      this->zone_config_[i].x1 = zones[i].x1;
      this->zone_config_[i].x2 = zones[i].x2;
      this->zone_config_[i].y1 = zones[i].y1;
      this->zone_config_[i].y2 = zones[i].y2;
    }

  } else {
    ESP_LOGW(TAG, "No zones found in preferences");
  }
}

void LD6001BComponent::dump_config() { ESP_LOGCONFIG(TAG, "HLK-LD6001B Human motion tracking radar module:");}

void LD6001BComponent::loop() {
  uint8_t byte;

  // Read data from the UART and push it to the frame parser
  while (this->available()) {
    if (this->read_byte(&byte)) {
      this->frame_parser_.push_data(byte);
    }
  }

  command_queue_.loop();
  update_sensors_();
}

void LD6001BComponent::start() {
  ESP_LOGW(TAG, "Starting HLK-LD6001B...");

  this->command_queue_.enqueue(
      Command::StartCommand([this](const std::string &response) { ESP_LOGW(TAG, "HLK-LD6001B started"); }));
}

void LD6001BComponent::stop() {
  ESP_LOGW(TAG, "Stopping HLK-LD6001B...");
  this->command_queue_.enqueue(
      Command::StopCommand([this](const std::string &response) { ESP_LOGW(TAG, "HLK-LD6001B stopped"); }));
}

void LD6001BComponent::soft_reset() {
  ESP_LOGD(TAG, "Resetting HLK-LD6001B...");
  this->command_queue_.enqueue(
      Command::ResetCommand([this](const std::string &response) { ESP_LOGD(TAG, "HLK-LD6001B reset"); }));
}

void LD6001BComponent::reset() {
  ESP_LOGE(TAG, "Setting NRST on HLK-LD6001B, needs power cycle after!");

  if (this->reset_pin_ != nullptr) {
    this->reset_pin_->pin_mode(gpio::FLAG_OUTPUT);
    this->reset_pin_->digital_write(true);
    ESP_LOGW(TAG, "HLK-LD6001B NRST pin is set");

    delay(100);
    this->reset_pin_->digital_write(false);
    ESP_LOGW(TAG, "HLK-LD6001B NRST pin is un set");
    delay(1000);
  }
}

void LD6001BComponent::config_factory_settings() {
  ESP_LOGW(TAG, "Configuring factory settings...");

  this->command_queue_.enqueue(Command::RestoreCommand([this](const std::string &response) {
    ESP_LOGW(TAG, "Factory settings configured");
    this->command_queue_.send(Command::ReadCommand());
  }));
}

void LD6001BComponent::set_protocol_mode(ProtocolMode mode) {
  ESP_LOGD(TAG, "Setting protocol mode to %d", mode);
  this->command_queue_.enqueue(Command::SetProtocolModeCommand(
      mode, [this, mode](const std::string &response) { ESP_LOGD(TAG, "Protocol mode set to %d", mode); }));
}

void LD6001BComponent::config_distance_sensitivity(uint8_t sensitivity) {
  ESP_LOGW(TAG, "Configuring distance sensitivity: %d", sensitivity);

  this->command_queue_.enqueue(
      Command::RangeSensitivityCommand(sensitivity, [this, sensitivity](const std::string &response) {
        ESP_LOGW(TAG, "Distance sensitivity configured: %d", sensitivity);
      }));
}

void LD6001BComponent::config_sensitivity(uint8_t sensitivity) {
  ESP_LOGW(TAG, "Configuring sensitivity: %d", sensitivity);

  this->command_queue_.enqueue(
      Command::SensitivityCommand(sensitivity, [this, sensitivity](const std::string &response) {
        ESP_LOGW(TAG, "Sensitivity configured: %d", sensitivity);
      }));
}

void LD6001BComponent::config_heartbeat_interval(uint16_t interval_s) {
  ESP_LOGW(TAG, "Configuring heartbeat interval: %d s", interval_s);

  this->command_queue_.enqueue(
      Command::HeartBeatIntervalCommand(interval_s, [this, interval_s](const std::string &response) {
        ESP_LOGW(TAG, "Heart beat interval configured: %d s", interval_s);
      }));
}

void LD6001BComponent::config_scan_interval(uint16_t interval_s) {
  ESP_LOGW(TAG, "Configuring scan interval: %d s", interval_s);

  this->command_queue_.enqueue(
      Command::ScanIntervalCommand(interval_s, [this, interval_s](const std::string &response) {
        ESP_LOGW(TAG, "Scan interval configured: %d s", interval_s);
      }));
}

void LD6001BComponent::config_monitor_interval(uint16_t interval_s) {
  ESP_LOGW(TAG, "Configuring monitor interval: %d s", interval_s);

  this->command_queue_.enqueue(
      Command::MonitorIntervalCommand(interval_s, [this, interval_s](const std::string &response) {
        ESP_LOGW(TAG, "Monitor interval configured: %d s", interval_s);
      }));
}

void LD6001BComponent::config_ground_radius(uint16_t radius_cm) {
  ESP_LOGW(TAG, "Configuring ground radius: %d cm", radius_cm);

  this->command_queue_.enqueue(Command::RangeCommand(radius_cm, [this, radius_cm](const std::string &response) {
    ESP_LOGW(TAG, "Ground radius configured: %d cm", radius_cm);
  }));
}

void LD6001BComponent::on_ack_response() { this->command_queue_.handleResponse(""); };

void LD6001BComponent::on_read_params_response(const ReadParamsResponse response) {
  ESP_LOGD(TAG, "READ response: Long Distance Sensitivity %d", response.range_sensitivity);
  maybe_publish(this->ground_radius_number_, response.range);
  maybe_publish(this->scan_interval_number_, response.time);
  maybe_publish(this->heartbeat_number_, response.heart_beat_interval);
  maybe_publish(this->monitor_interval_number_, response.monitor_time);
  maybe_publish(this->sensitivity_number_, heartbeat.sensitivity);
};

void LD6001BComponent::on_hear_beat_response(const HeartBeat heartbeat) {
  ESP_LOGD(TAG, "HeartBeat response: install method %2x", heartbeat.method);
  maybe_publish(this->ground_radius_number_, heartbeat.range);
  maybe_publish(this->heartbeat_number_, heartbeat.heart_beat_interval);
  maybe_publish(this->scan_interval_number_, heartbeat.scan_interval);
  maybe_publish(this->monitor_interval_number_, heartbeat.monitor_interval);
  maybe_publish(this->sensitivity_number_, heartbeat.sensitivity);
}

void LD6001BComponent::on_simple_radar_response(const std::vector<Person> people) {
  this->detailed_people_response_ = people;
  this->people_counted_ = people.size();
  ESP_LOGV(TAG, "Simple radar response: %d people detected", people_counted);
  this->target_tracker_.update(people);
}

void LD6001BComponent::on_detailed_radar_response(const std::vector<Person> people) {
  this->detailed_people_response_ = people;
  this->people_counted_ = people.size();
  ESP_LOGV(TAG, "Detailed radar response: %d people detected", this->people_counted_);
  this->target_tracker_.update(people);
}

void LD6001BComponent::on_invalid_frame() { ESP_LOGE(TAG, "Invalid frame received"); }

void LD6001BComponent::update_sensors_() {
  /*
      Reduce data update rate to prevent home assistant database size grow fast
    */
  int32_t current_millis = millis();
  if (current_millis - last_periodic_millis_ < this->throttle_) {
    return;
  }

  last_periodic_millis_ = current_millis;

  maybe_publish(this->target_count_sensor_, this->people_counted_);

  // Update sensors with the latest data
  for (size_t i = 0; i < MAX_TARGETS; i++) {
    float x = NAN;
    float y = NAN;
    float z = NAN;
    float distance = NAN;

    if (i < people_counted_) {
      auto person = this->detailed_people_response_[i];
      x = (person.x - 127) / 10 * 100;
      y = (person.y - 127) / 10 * 100;
      z = (person.z - 127) / 10 * 100;
      distance = sqrt(x * x + y * y + z * z);
    }

    maybe_publish(this->move_x_sensors_[i], x);
    maybe_publish(this->move_y_sensors_[i], y);
    maybe_publish(this->move_z_sensors_[i], z);
    maybe_publish(this->move_distance_sensors_[i], distance);
  }

  for (size_t index = 0; index < MAX_ZONES; ++index) {
    auto &zone = this->zone_config_[index];
    zone.target_count = 0;

    for (const auto &person : this->detailed_people_response_) {
      auto target_id = person.id;
      auto coord_x = person.x;
      auto coord_y = person.y;

      if (zone.contains(coord_x * 100, coord_y * 100)) {
        zone.target_count++;
      }
    }
    maybe_publish(this->zone_target_count_sensors_[index], zone.target_count);
  }

  this->update_trigger_.trigger(this->detailed_people_response_);
}

void LD6001BComponent::on_target_enter(uint32_t target_id) {
  ESP_LOGW(TAG, "Target %d entered view", target_id);
  this->target_enter_trigger_.trigger(target_id);
}

void LD6001BComponent::on_target_left(uint32_t target_id, uint32_t dwell_time) {
  ESP_LOGW(TAG, "Target %d left view, dwell time: %d seconds", target_id, dwell_time);
  this->target_left_trigger_.trigger(target_id, dwell_time);
}

#ifdef USE_SENSOR
void LD6001BComponent::set_move_x_sensor(uint8_t target, sensor::Sensor *s) { this->move_x_sensors_[target] = s; }
void LD6001BComponent::set_move_y_sensor(uint8_t target, sensor::Sensor *s) { this->move_y_sensors_[target] = s; }
void LD6001BComponent::set_move_z_sensor(uint8_t target, sensor::Sensor *s) { this->move_z_sensors_[target] = s; }
void LD6001BComponent::set_move_distance_sensor(uint8_t zone, sensor::Sensor *s) {
  this->move_distance_sensors_[zone] = s;
}
void LD6001BComponent::set_zone_target_count_sensor(uint8_t zone, sensor::Sensor *s) {
  this->zone_target_count_sensors_[zone] = s;
}
#endif

#ifdef USE_NUMBER
void LD6001BComponent::set_zone_coordinate(uint8_t zone) {
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

  ZoneCoordinates zones[MAX_ZONES];
  for (size_t i = 0; i < MAX_ZONES; i++) {
    zones[i].x1 = this->zone_config_[i].x1;
    zones[i].y1 = this->zone_config_[i].y1;
    zones[i].x2 = this->zone_config_[i].x2;
    zones[i].y2 = this->zone_config_[i].y2;
  }
  this->pref_.save(&zones);
}

void LD6001BComponent::set_zone_numbers(uint8_t zone, number::Number *x1, number::Number *y1, number::Number *x2,
                                       number::Number *y2) {
  if (zone < MAX_ZONES) {
    this->zone_numbers_[zone].x1 = x1;
    this->zone_numbers_[zone].y1 = y1;
    this->zone_numbers_[zone].x2 = x2;
    this->zone_numbers_[zone].y2 = y2;
  }
}
#endif

}  // namespace LD6001B
}  // namespace esphome
