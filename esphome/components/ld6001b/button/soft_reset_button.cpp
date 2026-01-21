#include "soft_reset_button.h"

namespace esphome {
namespace ld6001b {

void SoftResetButton::press_action() { this->parent_->soft_reset(); }

}  // namespace ld6001b
}  // namespace esphome
