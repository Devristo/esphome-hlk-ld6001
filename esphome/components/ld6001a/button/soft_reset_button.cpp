#include "soft_reset_button.h"

namespace esphome {
namespace ld6001a {

void SoftResetButton::press_action() { this->parent_->soft_reset(); }

}  // namespace ld6001a
}  // namespace esphome
