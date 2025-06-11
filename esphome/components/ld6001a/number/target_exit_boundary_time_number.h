#pragma once

#include "esphome/components/number/number.h"
#include "../ld6001a.h"

namespace esphome { 
namespace ld6001a {

class TargetExitBoundaryTimeNumber : public number::Number, public Parented<LD6001AComponent> {
 public:
  TargetExitBoundaryTimeNumber();

 protected:
  void control(float value) override;
};

}  // namespace ld2410
}  // namespace esphome
