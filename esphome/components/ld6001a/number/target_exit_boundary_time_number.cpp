#include "target_exit_boundary_time_number.h"

namespace esphome {
namespace ld6001a {

TargetExitBoundaryTimeNumber::TargetExitBoundaryTimeNumber() {}

void TargetExitBoundaryTimeNumber::control(float value) {
  this->publish_state(value);
  this->parent_->config_exit_boundary_time(value * 1000);
}

}  // namespace ld6001a
}  // namespace esphome
