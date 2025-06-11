#include "static_target_disappearance_time_number.h"

namespace esphome {
namespace ld6001a {

StaticTargetDisappearanceTimeNumber::StaticTargetDisappearanceTimeNumber() {}

void StaticTargetDisappearanceTimeNumber::control(float value) {
  this->publish_state(value);
  this->parent_->config_static_target_disappearance_time(value * 1000);
}

}  // namespace ld6001a
}  // namespace esphome
