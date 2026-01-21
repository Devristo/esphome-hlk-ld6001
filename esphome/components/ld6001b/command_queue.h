#pragma once

#include <queue>
#include <string>
#include <functional>

namespace esphome::ld6001b {
enum ProtocolMode {
  PROTOCOL_MODE_SIMPLE = 0,
  PROTOCOL_MODE_OUTPUT_STRING = 1,
  PROTOCOL_MODE_DEBUG = 2
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

  static Command ScanIntervalCommand(uint16_t interval_s, std::function<void(const std::string &)> cb = nullptr) {
    assert(interval_s >= 100 && interval_s <= 10000);

    return Command(str_sprintf("AT+TIME=%d\n", interval_s).c_str(), cb);
  }

  static Command MonitorIntervalCommand(uint16_t interval_s, std::function<void(const std::string &)> cb = nullptr) {
    assert(interval_s >= 1 && interval_s <= 99);

    return Command(str_sprintf("AT+MONTIME=%d\n", interval_s).c_str(), cb);
  }

  static Command HeartBeatIntervalCommand(uint16_t interval_s, std::function<void(const std::string &)> cb = nullptr) {
    assert(interval_s >= 10 && interval_s <= 999);

    return Command(str_sprintf("AT+HEATIME=%d\n", interval_s).c_str(), cb);
  }

  static Command InstallationHeightCommand(uint16_t height_cm, std::function<void(const std::string &)> cb = nullptr) {
    assert(height_cm >= 50 && height_cm <= 500);
    return Command(str_sprintf("AT+HEIGHT=%d\n", height_cm).c_str(), cb);
  }

  static Command RangeSensitivityCommand(uint16_t sensitivity, std::function<void(const std::string &)> cb = nullptr) {
    assert(sensitivity >= 1 && sensitivity <= 9);
    return Command(str_sprintf("AT+DPKTH=%d\n", sensitivity).c_str(), cb);
  }

  static Command SensitivityCommand(uint16_t sensitivity, std::function<void(const std::string &)> cb = nullptr) {
    assert(sensitivity >= 1 && sensitivity <= 19);
    return Command(str_sprintf("AT+SENS=%d\n", sensitivity).c_str(), cb);
  }

  static Command SetProtocolModeCommand(ProtocolMode mode, std::function<void(const std::string &)> cb = nullptr) {
    return Command(str_sprintf("AT+DEBUG=%d\n", mode).c_str(), cb);
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
      ESP_LOGE("ld6001b", "Previous command timed out.");
    }

    if (!waitingForAck && !queue.empty() && sendFunction) {
      const Command &cmd = queue.front();
      sendFunction(cmd.data);
      waitingForAck = currentMillis;
    }
  }
};

}  // namespace esphome::ld6001b