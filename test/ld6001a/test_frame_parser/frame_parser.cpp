#define UNITY_INCLUDE_PRINT_FORMATTED 1

#include "unity.h"
#include <queue>
#include <vector>
#include "ld6001a/frame_parser.h"  // Include the header file for the class being tested

#include "esphome/components/json/json_util.cpp" // Ugly, but otherwise the test fails to compile with "undefined reference to `esphome::json::parse_json(std::string const&, esphome::json::json_parse_t const&)'"
#include <ArduinoFake.h>

using namespace esphome::ld6001a;

void test_parser_should_start_in_idle_state(void) {
  class InlineFrameHandler : public FrameHandler {
  };

  InlineFrameHandler handler;
  FrameParser frame_iterator = FrameParser(handler);

  TEST_ASSERT_EQUAL(ParseState::IDLE, frame_iterator.state_);
}

void test_it_should_accept_at_ok(void) {
  class InlineFrameHandler : public FrameHandler {
    public:
      bool on_ack_response_called = false;

      void on_ack_response() {
        on_ack_response_called = true;
      }
  };

  InlineFrameHandler handler;
  FrameParser frame_iterator = FrameParser(handler);
  TEST_ASSERT_EQUAL(ParseState::IDLE, frame_iterator.state_);
  frame_iterator.push_data('A');
  frame_iterator.push_data('T');
  frame_iterator.push_data('+');
  frame_iterator.push_data('O');
  frame_iterator.push_data('K');
  frame_iterator.push_data('\r');
  frame_iterator.push_data('\n');

  TEST_ASSERT_EQUAL(ParseState::IDLE, frame_iterator.state_);
  TEST_ASSERT_EQUAL(true, handler.on_ack_response_called);
}

void test_it_should_skip_unknown_bytes(void) {
  class InlineFrameHandler : public FrameHandler {
    public:
      bool on_ack_response_called = false;

      void on_ack_response() {
        on_ack_response_called = true;
      }
  };

  InlineFrameHandler handler;
  FrameParser frame_iterator = FrameParser(handler);
  TEST_ASSERT_EQUAL(ParseState::IDLE, frame_iterator.state_);
  frame_iterator.push_data('A');
  frame_iterator.push_data('T');
  TEST_ASSERT_EQUAL(ParseState::READING_HEADER, frame_iterator.state_);
  frame_iterator.push_data('-');
  TEST_ASSERT_EQUAL(ParseState::INVALID, frame_iterator.state_);
  frame_iterator.push_data('-');
  TEST_ASSERT_EQUAL(ParseState::INVALID, frame_iterator.state_);
  frame_iterator.push_data('A');
  TEST_ASSERT_EQUAL(ParseState::READING_HEADER, frame_iterator.state_);
  frame_iterator.push_data('T');
  frame_iterator.push_data('+');
  frame_iterator.push_data('O');
  frame_iterator.push_data('K');
  frame_iterator.push_data('\r');
  frame_iterator.push_data('\n');

  TEST_ASSERT_EQUAL(ParseState::IDLE, frame_iterator.state_);
  TEST_ASSERT_EQUAL(true, handler.on_ack_response_called);
}

void test_it_should_accept_binary_type1(void) {
  class InlineFrameHandler : public FrameHandler {
    public:
      int people_counted = -1;

      void on_simple_radar_response(const uint8_t people_counted) override {
        this->people_counted = people_counted;
      }
  };

  InlineFrameHandler handler;
  FrameParser frame_iterator = FrameParser(handler);
  frame_iterator.push_data(0x55);
  frame_iterator.push_data(0xAA);
  frame_iterator.push_data(0x0A);
  frame_iterator.push_data(0x04);
  TEST_ASSERT_EQUAL(ParseState::READING_HEADER, frame_iterator.state_);
  frame_iterator.push_data(0x00);
  frame_iterator.push_data(0x00);
  frame_iterator.push_data(0x00);
  frame_iterator.push_data(0x00);
  frame_iterator.push_data(0x00);
  frame_iterator.push_data(0x0E);

  TEST_ASSERT_EQUAL(0, handler.people_counted);
  TEST_ASSERT_EQUAL(ParseState::IDLE, frame_iterator.state_);
}

void test_it_should_accept_binary_type2(void) {
  class InlineFrameHandler : public FrameHandler {
    public:
      std::vector<Person> people;

      void on_detailed_radar_response(const std::vector<Person> people) override {
        this->people = people;
      }
  };

  InlineFrameHandler handler;
  FrameParser frame_iterator = FrameParser(handler);
  // 0 Bytes
  frame_iterator.push_data<8>({0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08});
  frame_iterator.push_uint32(96); // Length of the body
  TEST_ASSERT_EQUAL(ParseState::READING_HEADER, frame_iterator.state_);
  frame_iterator.push_data<4>({0xA3, 0x01, 0x00, 0x00}); // Length of the body
  frame_iterator.push_data<4>({0x01, 0x00, 0x00, 0x00}); // TLV1
  frame_iterator.push_data<4>({0x0, 0x00, 0x00, 0x00}); // Constant
  frame_iterator.push_data<4>({0x02, 0x00, 0x00, 0x00}); // TLV2
  frame_iterator.push_data<4>({0x40, 0x00, 0x00, 0x00}); // TRACKLENGTH (number of people * 32)
  TEST_ASSERT_EQUAL(ParseState::READING_HEADER, frame_iterator.state_);
  // 32 bytes

  // Person 0
  frame_iterator.push_data<4>({0x0, 0x0, 0x0, 0x0}); // Reserved constant
  frame_iterator.push_data<4>({0x0, 0x0, 0x0, 0x0}); // Person ID
  frame_iterator.push_data<4>({0x21, 0x28, 0x96, 0xBF}); // X (float)
  frame_iterator.push_data<4>({0xCB, 0x85, 0x20, 0x40}); // Y (float)
  frame_iterator.push_data<4>({0x9A, 0xAB, 0xA3, 0x3E}); // Z (float)
  TEST_ASSERT_EQUAL(ParseState::READING_HEADER, frame_iterator.state_);
    
  frame_iterator.push_data<4>({0x8A, 0xBD, 0xC1, 0x3D}); // Vx (float)
  frame_iterator.push_data<4>({0x50, 0x98, 0x99, 0xBD}); // Vy (float)
  frame_iterator.push_data<4>({0x40, 0x52, 0xC3, 0x3A}); // Vz (float)
  // 64 bytes

  // Person 1
  frame_iterator.push_data<4>({0x0, 0x0, 0x0, 0x0}); // Reserved constant
  frame_iterator.push_data<4>({0x1, 0x0, 0x0, 0x0}); // Person 2
  frame_iterator.push_data<4>({0x21, 0x28, 0x96, 0xBF}); // X (float)
  frame_iterator.push_data<4>({0xCB, 0x85, 0x20, 0x40}); // Y (float)
  frame_iterator.push_data<4>({0x9A, 0xAB, 0xA3, 0x3E}); // Z (float)
  TEST_ASSERT_EQUAL(ParseState::READING_HEADER, frame_iterator.state_);
    
  frame_iterator.push_data<4>({0x8A, 0xBD, 0xC1, 0x3D}); // Vx (float)
  frame_iterator.push_data<4>({0x50, 0x98, 0x99, 0xBD}); // Vy (float)
  frame_iterator.push_data<4>({0x40, 0x52, 0xC3, 0x3A}); // Vz (float)
  // 96 bytes

  frame_iterator.push_data(0xA3); // Checksum
  TEST_ASSERT_EQUAL(ParseState::IDLE, frame_iterator.state_);
  TEST_ASSERT_EQUAL(2, handler.people.size());
}

void test_it_should_validate_checksum(void) {
  class InlineFrameHandler : public FrameHandler {
    public:
      int people_counted = -1;

      void on_simple_radar_response(const uint8_t people_counted) override {
        this->people_counted = people_counted;
      }
  };

  InlineFrameHandler handler;
  FrameParser frame_iterator = FrameParser(handler);
  frame_iterator.push_data<2>({0x55, 0xAA});
  frame_iterator.push_data(10); // Length of the body
  frame_iterator.push_data(0x04);
  TEST_ASSERT_EQUAL(ParseState::READING_HEADER, frame_iterator.state_);
  frame_iterator.push_data<2>({0x00, 0x00});
  frame_iterator.push_data(0x00);
  frame_iterator.push_data(0x00);
  frame_iterator.push_data(0x04); // This will make the checksum invalid
  frame_iterator.push_data(0x0E);

  TEST_ASSERT_EQUAL(-1, handler.people_counted);
  TEST_ASSERT_EQUAL(ParseState::INVALID, frame_iterator.state_);
}

int runUnityTests(void) {
  UNITY_BEGIN();
  RUN_TEST(test_parser_should_start_in_idle_state);
  RUN_TEST(test_it_should_skip_unknown_bytes);
  RUN_TEST(test_it_should_accept_at_ok);
  RUN_TEST(test_it_should_accept_binary_type1);
  RUN_TEST(test_it_should_accept_binary_type2);
  RUN_TEST(test_it_should_validate_checksum);
  return UNITY_END();
}

// WARNING!!! PLEASE REMOVE UNNECESSARY MAIN IMPLEMENTATIONS //

/**
 * For native dev-platform or for some embedded frameworks
 */
int main(void) { 
  return runUnityTests(); 
}
