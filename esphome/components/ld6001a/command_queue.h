#pragma once

#include <queue>
#include <string>
#include <functional>

namespace esphome::ld6001a {
enum ProtocolMode {
  PROTOCOL_MODE_SIMPLE = 0,
  PROTOCOL_MODE_OUTPUT_STRING = 1,
  PROTOCOL_MODE_DEBUG = 2,
  PROTOCOL_MODE_DETAILED = 3,
};
class Command {
 public:
  std::string data;
  std::function<void(const std::string &)> onResponse;

  Command(const std::string &cmd, std::function<void(const std::string &)> cb = nullptr) : data(cmd), onResponse(cb) {}

  static Command ReadCommand(std::function<void(const std::string &)> cb = nullptr) { return Command("AT+READ\n", cb); }

  static Command StartCommand(std::function<void(const std::string &)> cb = nullptr) {
    return Command("AT+START\n", cb);
  }

  static Command StopCommand(std::function<void(const std::string &)> cb = nullptr) { return Command("AT+STOP\n", cb); }

  static Command ResetCommand(std::function<void(const std::string &)> cb = nullptr) {
    return Command("AT+RESET\n", cb);
  }

  static Command RestoreCommand(std::function<void(const std::string &)> cb = nullptr) {
    return Command("AT+RESTORE\n", cb);
  }

  static Command RangeCommand(uint16_t radius_cm, std::function<void(const std::string &)> cb = nullptr) {
    assert(radius_cm >= 100 && radius_cm <= 500);

    return Command(str_sprintf("AT+RANGE=%d\n", radius_cm).c_str(), cb);
  }

  static Command TargetExitBoundaryTimeCommand(uint32_t time_ms,
                                               std::function<void(const std::string &)> cb = nullptr) {
    assert(time_ms % 100 == 0);
    assert(time_ms >= 200 && time_ms <= 1000 * 100);

    return Command(str_sprintf("AT+Exit=%d\n", time_ms / 100).c_str(), cb);
  }

  static Command HeartBeatIntervalCommand(uint16_t interval_s, std::function<void(const std::string &)> cb = nullptr) {
    assert(interval_s >= 10 && interval_s <= 999);

    return Command(str_sprintf("AT+HEATIME=%d\n", interval_s).c_str(), cb);
  }

  static Command InstallationHeightCommand(uint16_t height_cm, std::function<void(const std::string &)> cb = nullptr) {
    assert(height_cm >= 50 && height_cm <= 500);
    return Command(str_sprintf("AT+HEIGHTD=%d\n", height_cm).c_str(), cb);
  }

  static Command RangeSensitivityCommand(uint16_t sensitivity, std::function<void(const std::string &)> cb = nullptr) {
    assert(sensitivity >= 1 && sensitivity <= 9);
    return Command(str_sprintf("AT+DPKTH=%d\n", sensitivity).c_str(), cb);
  }

  static Command XMinCommand(int x_min, std::function<void(const std::string &)> cb = nullptr) {
    assert(x_min >= -500 && x_min <= -20);
    return Command(str_sprintf("AT+XNega=%d\n", x_min).c_str(), cb);
  }

  static Command XMaxCommand(int x_max, std::function<void(const std::string &)> cb = nullptr) {
    assert(x_max >= 20 && x_max <= 500);
    return Command(str_sprintf("AT+XPosi=%d\n", x_max).c_str(), cb);
  }

  static Command YMinCommand(int y_min, std::function<void(const std::string &)> cb = nullptr) {
    assert(y_min >= -500 && y_min <= -20);
    return Command(str_sprintf("AT+YNega=%d\n", y_min).c_str(), cb);
  }

  static Command YMaxCommand(int y_max, std::function<void(const std::string &)> cb = nullptr) {
    assert(y_max >= 20 && y_max <= 500);
    return Command(str_sprintf("AT+YPosi=%d\n", y_max).c_str(), cb);
  }

  static Command SetProtocolModeCommand(ProtocolMode mode, std::function<void(const std::string &)> cb = nullptr) {
    return Command(str_sprintf("AT+DEBUG=%d\n", mode).c_str(), cb);
  }

  static Command SetMovingTargetDisappearanceTimeCommand(uint32_t time_ms,
                                                         std::function<void(const std::string &)> cb = nullptr) {
    assert(time_ms % 100 == 0);
    assert(time_ms >= 500 && time_ms <= 1000 * 100);

    return Command(str_sprintf("AT+Moving=%d\n", time_ms / 100).c_str(), cb);
  }
  static Command SetStaticTargetDisappearanceTimeCommand(uint32_t time_ms,
                                                         std::function<void(const std::string &)> cb = nullptr) {
    assert(time_ms % 100 == 0);
    assert(time_ms >= 500 && time_ms <= 1000 * 100);

    return Command(str_sprintf("AT+Static=%d\n", time_ms / 100).c_str(), cb);
  }
};

class CommandQueue {
 public:
  CommandQueue(std::function<void(const std::string &)> sender) : sendFunction(sender) {};

  void enqueue(const Command &cmd) {
    queue.push(cmd);
    trySendNext();
  }

  void send(const Command &cmd) {
    if (sendFunction) {
      sendFunction(cmd.data);
    }
  }

  void loop() {
    trySendNext();
  }

  void handleResponse(const std::string &response) {
    if (!waitingForAck || queue.empty())
      return;

    Command &cmd = queue.front();
    if (cmd.onResponse) {
      cmd.onResponse(response);
    }

    queue.pop();
    waitingForAck = 0;
    trySendNext();
  }

 private:
  std::queue<Command> queue;
  std::function<void(const std::string &)> sendFunction;
  uint32_t waitingForAck = 0;

  void trySendNext() {
    auto currentMillis = millis();

    // Timeout command after 1 second if no ack is received
    if (waitingForAck != 0 && waitingForAck + 5000 < currentMillis) {
      waitingForAck = 0;
      ESP_LOGE("ld6001a", "Previous command timed out.");
    }

    if (!waitingForAck && !queue.empty() && sendFunction) {
      const Command &cmd = queue.front();
      sendFunction(cmd.data);
      waitingForAck = currentMillis;
    }
  }
};

}  // namespace esphome::ld6001a