#pragma once

#include "esphome/components/number/number.h"
#include "../ld6001a.h"

namespace esphome {
namespace ld6001a {

class LongDistanceSensitivityNumber : public number::Number, public Parented<LD6001AComponent> {
 public:
  LongDistanceSensitivityNumber();

 protected:
  void control(float value) override;
};

}  // namespace ld2410
}  // namespace esphome
