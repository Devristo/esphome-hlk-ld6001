#pragma once

#include "esphome/components/button/button.h"
#include "../ld6001b.h"

namespace esphome {
namespace ld6001b {

class SoftResetButton : public button::Button, public Parented<LD6001BComponent> {
 public:
  SoftResetButton() = default;

 protected:
  void press_action() override;
};

}  // namespace ld6001b
}  // namespace esphome
