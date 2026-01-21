#include "scan_interval_number.h"

namespace esphome {
namespace ld6001b {

ScanIntervalNumber::ScanIntervalNumber() {}

void ScanIntervalNumber::control(float value) {
  this->publish_state(value);
  this->parent_->config_heartbeat_interval(value);
}

}  // namespace ld6001b
}  // namespace esphome
