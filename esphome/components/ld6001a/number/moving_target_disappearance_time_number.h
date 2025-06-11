#pragma once

#include "esphome/components/number/number.h"
#include "../ld6001a.h"

namespace esphome {
namespace ld6001a {

class MovingTargetDisappearanceTimeNumber : public number::Number, public Parented<LD6001AComponent> {
 public:
  MovingTargetDisappearanceTimeNumber();

 protected:
  void control(float value) override;
};

}  // namespace ld2410
}  // namespace esphome
