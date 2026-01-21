#pragma once

#include "esphome/components/button/button.h"
#include "../ld6001a.h"

namespace esphome {
namespace ld6001a {

class FactoryResetButton : public button::Button, public Parented<LD6001AComponent> {
 public:
  FactoryResetButton() = default;

 protected:
  void press_action() override;
};

}  // namespace ld6001a
}  // namespace esphome
