#pragma once

#include "esphome/components/button/button.h"
#include "../ld6001b.h"

namespace esphome {
namespace ld6001b {

class StartButton : public button::Button, public Parented<LD6001BComponent> {
 public:
  StartButton() = default;

 protected:
  void press_action() override;
};

}  // namespace ld6001b
}  // namespace esphome
