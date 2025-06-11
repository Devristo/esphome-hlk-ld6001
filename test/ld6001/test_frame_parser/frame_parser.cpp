#define UNITY_INCLUDE_PRINT_FORMATTED 1

#include "unity.h"
#include <queue>
#include <vector>
#include "ld6001/frame_parser.h"  // Include the header file for the class being tested
#include <ArduinoFake.h>

using namespace esphome::ld6001;

void write_with_checksum(FrameParser &parser, std::vector<uint8_t> buffer) {
  for (uint8_t c : buffer) {
    parser.push_data(c);
  }

  parser.push_data(FrameParser::get_iterator_checksum(buffer.begin(), buffer.end()));
  parser.push_data(0x4A);
}

void test_it_should_parse_status_response(void) {
  class InlineFrameHandler : public FrameHandler {
    public:
      StatusResponse response{};

      void on_status_response(const StatusResponse &response) override {
        this->response = response;
      }
  };

  InlineFrameHandler handler;
  FrameParser frame_iterator = FrameParser(handler);
  frame_iterator.push_data(0x4D);
  frame_iterator.push_data(0x11);
  frame_iterator.push_data(0x08);
  frame_iterator.push_data(0x00);
  frame_iterator.push_data(0x01);
  frame_iterator.push_data(0x02);
  frame_iterator.push_data(0x03);
  frame_iterator.push_data(0x04);
  frame_iterator.push_data(0x00);
  frame_iterator.push_data(0x01);
  frame_iterator.push_data(0x00);
  frame_iterator.push_data(0x00);
  frame_iterator.push_data(113);
  frame_iterator.push_data(0x4A);

  TEST_ASSERT_EQUAL(0x01, handler.response.software_version_minor);
  TEST_ASSERT_EQUAL(0x02, handler.response.software_version_major);
  TEST_ASSERT_EQUAL(0x03, handler.response.hardware_version_minor);
  TEST_ASSERT_EQUAL(0x04, handler.response.hardware_version_major);
}

void test_it_should_parse_radar_response(void) {
  class InlineFrameHandler : public FrameHandler {
    public:
      RadarResponse response{};

      void on_radar_response(const RadarResponse &response) override {
        this->response = response;
      }
  };

  InlineFrameHandler handler;
  FrameParser frame_iterator = FrameParser(handler);

  write_with_checksum(
    frame_iterator,
    std::vector<unsigned char>{
      0x4D,
      0x62,
      16, // (targets+1) * 8
      0x00,
      0x00,
      0x01,

      0x00,
      0x00,
      0x00,
      0x00,
      0x00,
      0x00,

      // Target 1
      0x01,
      255,
      120,
      90,
      0x00,
      0x00,
      static_cast<uint8_t>(-100),
      static_cast<uint8_t>(-110),
    }
  );

  TEST_ASSERT_EQUAL(0x00, handler.response.fault_status);
  TEST_ASSERT_EQUAL(0x01, handler.response.targets);
  
  // Target 1 assertions
  TEST_ASSERT_EQUAL(0x01, handler.response.people[0].id);
  TEST_ASSERT_EQUAL(2550, handler.response.people[0].distance);
  TEST_ASSERT_EQUAL(120, handler.response.people[0].pitch_angle);
  TEST_ASSERT_EQUAL(90, handler.response.people[0].horizontal_angle);
  TEST_ASSERT_EQUAL(-1000, handler.response.people[0].x);
  TEST_ASSERT_EQUAL(-1100, handler.response.people[0].y);
}

int runUnityTests(void) {
  UNITY_BEGIN();
  RUN_TEST(test_it_should_parse_status_response);
  RUN_TEST(test_it_should_parse_radar_response);
  return UNITY_END();
}

// WARNING!!! PLEASE REMOVE UNNECESSARY MAIN IMPLEMENTATIONS //

/**
 * For native dev-platform or for some embedded frameworks
 */
int main(void) { 
  return runUnityTests(); 
}
