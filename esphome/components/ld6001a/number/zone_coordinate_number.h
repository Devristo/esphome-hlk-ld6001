#pragma once

#include "esphome/components/number/number.h"
#include "../ld6001a.h"

namespace esphome {
namespace ld6001a {

class ZoneCoordinateNumber : public number::Number, public Parented<LD6001AComponent> {
 public:
  ZoneCoordinateNumber(uint8_t zone);

 protected:
  uint8_t zone_;
  void control(float value) override;
};

}  // namespace ld6001
}  // namespace esphome
