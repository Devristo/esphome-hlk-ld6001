#pragma once

#include "esphome/components/number/number.h"
#include "../ld6001b.h"

namespace esphome {
namespace ld6001b {

class ZoneCoordinateNumber : public number::Number, public Parented<LD6001BComponent> {
 public:
  ZoneCoordinateNumber(uint8_t zone);

 protected:
  uint8_t zone_;
  void control(float value) override;
};

}  // namespace ld6001b
}  // namespace esphome
