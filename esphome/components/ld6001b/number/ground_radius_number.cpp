#include "ground_radius_number.h"

namespace esphome {
namespace ld6001b {

GroundRadiusNumber::GroundRadiusNumber() {}

void GroundRadiusNumber::control(float value) {
  this->publish_state(value);
  this->parent_->config_ground_radius(value);
}

}  // namespace ld6001b
}  // namespace esphome
