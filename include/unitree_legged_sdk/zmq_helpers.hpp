#include <boost/algorithm/string/split.hpp>
#include <boost/algorithm/string/trim.hpp>
#include <chrono>
#include <iostream>
#include <string>
#include <vector>
#include <zmq.hpp>

#include "unitree_legged_sdk/unitree_legged_sdk.h"

using namespace UNITREE_LEGGED_SDK;

// Shortcuts

using std::cout;
using std::endl;
using std::string;
using std::vector;

// Configurations

const int io_threads = 1;
const int msg_hz = 2;
const int msg_size = 128;
const int msg_gap = 1000 / msg_hz;  // In milliseconds
const int msg_ttl = msg_gap / 2;    // In the number of control messages

// Gets the current time in milliseconds.
int64_t get_epoch_ms() {
  auto current = std::chrono::system_clock::now().time_since_epoch();
  return std::chrono::duration_cast<std::chrono::milliseconds>(current).count();
}

// Convert a ZMQ message to string fields.
void msg_to_fields(zmq::message_t &msg, vector<string> &vec) {
  string str = string(static_cast<char *>(msg.data()), msg.size());
  boost::trim(str);
  boost::split(vec, str, boost::is_any_of(" "));
}

// Convert state to a ZMQ message.
void state_to_msg(zmq::message_t &msg, HighState &state) {
  snprintf((char *)msg.data(), msg_size, "stat %ld %f %f %f %f %f %f %f %f %f %f ",

           get_epoch_ms(),

           state.imu.quaternion[0], state.imu.quaternion[1], state.imu.quaternion[2],
           state.imu.quaternion[3],

           state.imu.gyroscope[0], state.imu.gyroscope[1], state.imu.gyroscope[2],

           state.imu.accelerometer[0], state.imu.accelerometer[1], state.imu.accelerometer[2]);
}

// Prepare a high-cmd with the given (linear x, linear y, angular z).
void set_cmd(HighCmd &cmd, float lin_x, float lin_y, float ang_z) {
  cmd.levelFlag = UNITREE_LEGGED_SDK::HIGHLEVEL;
  cmd.mode = 0;
  cmd.gaitType = 0;
  cmd.speedLevel = 0;
  cmd.footRaiseHeight = 0;
  cmd.bodyHeight = 0;
  cmd.euler[0] = 0;
  cmd.euler[1] = 0;
  cmd.euler[2] = 0;
  cmd.velocity[0] = 0.0f;
  cmd.velocity[1] = 0.0f;
  cmd.yawSpeed = 0.0f;
  cmd.reserve = 0;

  cmd.velocity[0] = lin_x;
  cmd.velocity[1] = lin_y;
  cmd.yawSpeed = ang_z;

  cmd.mode = 2;
  cmd.gaitType = 1;
}
