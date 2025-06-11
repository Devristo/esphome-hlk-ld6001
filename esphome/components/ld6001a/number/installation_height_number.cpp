#include "installation_height_number.h"

namespace esphome {
namespace ld6001a {

InstallationHeightNumber::InstallationHeightNumber() {}

void InstallationHeightNumber::control(float value) {
  this->publish_state(value);
  this->parent_->config_vertical_distance(value);
}

}  // namespace ld6001a
}  // namespace esphome
