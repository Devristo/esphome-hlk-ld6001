#pragma once

#include "esphome/components/button/button.h"
#include "../ld6001a.h"

namespace esphome {
namespace ld6001a {

class StopButton : public button::Button, public Parented<LD6001AComponent> {
 public:
  StopButton() = default;

 protected:
  void press_action() override;
};

}  // namespace ld6001a
}  // namespace esphome
