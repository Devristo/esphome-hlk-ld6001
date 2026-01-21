#include "monitor_interval_number.h"

namespace esphome {
namespace ld6001b {

MonitorIntervalNumber::MonitorIntervalNumber() {}

void MonitorIntervalNumber::control(float value) {
  this->publish_state(value);
  this->parent_->config_monitor_interval(value);
}

}  // namespace ld6001b
}  // namespace esphome
