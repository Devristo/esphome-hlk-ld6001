#include "start_button.h"

namespace esphome {
namespace ld6001b {

void StartButton::press_action() { this->parent_->start(); }

}  // namespace ld6001b
}  // namespace esphome
