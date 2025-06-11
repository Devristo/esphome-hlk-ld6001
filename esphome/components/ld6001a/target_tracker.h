#pragma once

#include <queue>
#include <cinttypes>
#include <unordered_map>
#include <unordered_set>

namespace esphome {
namespace ld6001a {

template<typename T>
class TargetEventHandler {
 public:
  virtual void on_target_enter(T target_id) = 0;
  virtual void on_target_left(T target_id, uint32_t dwell_time) = 0;
};

template<typename T>
class TargetTracker {
  using id_type = decltype(std::declval<T>().id);

 public:
  TargetTracker(TargetEventHandler<id_type> &event_handler) : event_handler_(event_handler) {}

  template<typename Container> void update(const Container &targets) {
    auto now = millis();
    auto unseen = std::unordered_set<id_type>();

    for (const auto &[key, _] : targets_) {
      unseen.insert(key);
    }

    for (const auto &target : targets) {
      unseen.erase(target.id);
      targets_[target.id] = target;

      if (entry_times_.find(target.id) == entry_times_.end()) {
        entry_times_[target.id] = now;
        event_handler_.on_target_enter(target.id);
      }

      last_seen_times_[target.id] = now;
    }

    for (const auto &target_id : unseen) {
      if (entry_times_.find(target_id) != entry_times_.end()) {
        uint32_t dwell_time = (now - entry_times_[target_id]) / 1000;
        event_handler_.on_target_left(target_id, dwell_time);
        entry_times_.erase(target_id);
        last_seen_times_.erase(target_id);
      }
    }
  }

 protected:
  std::unordered_map<id_type, T> targets_;
  TargetEventHandler<id_type> &event_handler_;
  std::unordered_map<id_type, uint32_t> entry_times_;
  std::unordered_map<id_type, uint32_t> last_seen_times_;
};
}  // namespace ld6001a
}  // namespace esphome