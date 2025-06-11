#pragma once

#include "esphome/components/number/number.h"
#include "../ld6001a.h"

namespace esphome {
namespace ld6001a {

enum CoordinateType {
  X_MIN = 0,
  X_MAX = 1,
  Y_MIN = 2,
  Y_MAX = 3,
};

class CoordinateNumber : public number::Number, public Parented<LD6001AComponent> {
 public:
  CoordinateNumber(CoordinateType type) : type_(type) {};

  // Additional constructor that accepts an int
  CoordinateNumber(int type_int) 
    : type_(static_cast<CoordinateType>(type_int)) {
  }

 protected:
  CoordinateType type_;
  void control(float value) override;
};

}  // namespace ld6001a
}  // namespace esphome
