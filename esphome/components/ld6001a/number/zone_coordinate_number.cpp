#include "zone_coordinate_number.h"

namespace esphome {
namespace ld6001a {

ZoneCoordinateNumber::ZoneCoordinateNumber(uint8_t zone) : zone_(zone) {}

void ZoneCoordinateNumber::control(float value) {
  this->publish_state(value);
  this->parent_->set_zone_coordinate(this->zone_);
}

}  // namespace ld6001a
}  // namespace esphome
