#pragma once

#include "esphome/components/button/button.h"
#include "../ld6001b.h"

namespace esphome {
namespace ld6001b {

class FactoryResetButton : public button::Button, public Parented<LD6001BComponent> {
 public:
  FactoryResetButton() = default;

 protected:
  void press_action() override;
};

}  // namespace ld6001b
}  // namespace esphome
