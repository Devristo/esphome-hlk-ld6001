#include "stop_button.h"

namespace esphome {
namespace ld6001b {

void StopButton::press_action() { this->parent_->stop(); }

}  // namespace ld6001b
}  // namespace esphome
