#pragma once

#include "esphome/components/number/number.h"
#include "../ld6001b.h"

namespace esphome {
namespace ld6001b {

class InstallationHeightNumber : public number::Number, public Parented<LD6001BComponent> {
 public:
  InstallationHeightNumber();

 protected:
  void control(float value) override;
};

}  // namespace ld6001b
}  // namespace esphome
