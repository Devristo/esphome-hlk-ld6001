#pragma once

#include "esphome/components/number/number.h"
#include "../ld6001a.h"

namespace esphome {
namespace ld6001a {

class StaticTargetDisappearanceTimeNumber : public number::Number, public Parented<LD6001AComponent> {
 public:
  StaticTargetDisappearanceTimeNumber();

 protected:
  void control(float value) override;
};

}  // namespace ld2410
}  // namespace esphome
