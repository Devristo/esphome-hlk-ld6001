#include "sensitivity_number.h"

namespace esphome {
namespace ld6001b {

SensitivityNumber::SensitivityNumber() {}

void SensitivityNumber::control(float value) {
  this->publish_state(value);
  this->parent_->config_sensitivity(value);
}

}  // namespace ld6001b
}  // namespace esphome
