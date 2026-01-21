#include "factory_reset_button.h"

namespace esphome {
namespace ld6001a {

void FactoryResetButton::press_action() { this->parent_->config_factory_settings(); }

}  // namespace ld6001a
}  // namespace esphome
