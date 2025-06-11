#include "heartbeat_number.h"

namespace esphome {
namespace ld6001a {

HeartBeatNumber::HeartBeatNumber() {}

void HeartBeatNumber::control(float value) {
  this->publish_state(value);
  this->parent_->config_heartbeat_interval(value);
}

}  // namespace ld6001a
}  // namespace esphome
