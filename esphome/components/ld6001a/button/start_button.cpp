#include "start_button.h"

namespace esphome {
namespace ld6001a {

void StartButton::press_action() { this->parent_->start(); }

}  // namespace ld6001a
}  // namespace esphome
