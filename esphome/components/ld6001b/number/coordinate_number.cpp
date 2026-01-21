#include "coordinate_number.h"

namespace esphome {
namespace ld6001b {

void CoordinateNumber::control(float value) {
  this->publish_state(value);

  switch (this->type_) {
    case X_MIN:
      this->parent_->config_x_min(value);
      break;
    case X_MAX:
      this->parent_->config_x_max(value);
      break;
    case Y_MIN:
      this->parent_->config_y_min(value);
      break;
    case Y_MAX:
      this->parent_->config_y_max(value);
      break;
  }
}

}  // namespace ld6001b
}  // namespace esphome
