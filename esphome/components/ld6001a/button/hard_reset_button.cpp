#include "hard_reset_button.h"

namespace esphome {
namespace ld6001a {

void HardResetButton::press_action() { this->parent_->reset(); }

}  // namespace ld6001a
}  // namespace esphome
