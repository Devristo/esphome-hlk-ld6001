#include "long_distance_sensitivity_number.h"

namespace esphome {
namespace ld6001a {

LongDistanceSensitivityNumber::LongDistanceSensitivityNumber() {}

void LongDistanceSensitivityNumber::control(float value) {
  this->publish_state(value);
  this->parent_->config_distance_sensitivity(value);
}

}  // namespace ld6001a
}  // namespace esphome
