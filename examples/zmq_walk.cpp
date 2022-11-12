/*****************************************************************
 Copyright (c) 2020, Unitree Robotics.Co.Ltd. All rights reserved.
******************************************************************/
/*****************************************************************
 Modified from example_walk.cpp by LukeLook (lukeluocn@outlook.com)
******************************************************************/

#include <unistd.h>

#include "unitree_legged_sdk/zmq_helpers.hpp"

class Custom {
 public:
  Custom()
      : safe(LeggedType::Go1),
        udp(8090, "192.168.123.161", 8082, sizeof(HighCmd), sizeof(HighState)),
        context(io_threads),
        publisher(context, zmq::socket_type::pub),
        subscriber(context, zmq::socket_type::sub) {
    udp.InitCmdData(cmd);

    // will throw error if fail to bind
    publisher.bind("tcp://*:5555");
    subscriber.bind("tcp://*:5556");
    subscriber.setsockopt(ZMQ_SUBSCRIBE, "ctrl", 4);
  }

  // callbacks

  void UDPRecv() { udp.Recv(); }
  void UDPSend() { udp.Send(); }
  void RobotControl();

  // members

  Safety safe;
  UDP udp;

  float dt = 0.002;  // delta time (2ms)

  HighCmd cmd = {0};      // command to control the robot
  HighState state = {0};  // state fed by the robot

  zmq::context_t context;  // must be listed before sockets!
  zmq::socket_t publisher;
  zmq::socket_t subscriber;

  float global_lin_x = 0;
  float global_lin_y = 0;
  float global_ang_z = 0;
  int age = 0;
};

void Custom::RobotControl() {
  // Get and send state
  {
    udp.GetRecv(state);
    zmq::message_t msg(msg_size);
    state_to_msg(msg, state);
    publisher.send(msg);
  }

  age++;
  float local_lin_x, local_lin_y, local_ang_z;
  if (age <= msg_ttl) {
    // Reuse previous instructions
    local_lin_x = global_lin_x;
    local_lin_y = global_lin_y;
    local_ang_z = global_ang_z;
  } else if (age <= 2 * msg_ttl) {
    // Decay previous instructions
    local_lin_x = ((2 * msg_ttl - age) / (float)msg_ttl) * global_lin_x;
    local_lin_y = ((2 * msg_ttl - age) / (float)msg_ttl) * global_lin_y;
    local_ang_z = ((2 * msg_ttl - age) / (float)msg_ttl) * global_ang_z;
  } else {
    // Stop moving
    local_lin_x = 0;
    local_lin_y = 0;
    local_ang_z = 0;
  }

  // Try getting control msg
  zmq::message_t msg(msg_size);
  if (subscriber.recv(&msg, ZMQ_DONTWAIT)) {
    vector<string> fields;
    msg_to_fields(msg, fields);

    // Expected message format: "ctrl {linear.x} {linear.y} {angular.z}"
    if (fields.size() == 4) {
      // Parse the control
      global_lin_x = std::atof(fields[1].c_str());
      global_lin_y = std::atof(fields[2].c_str());
      global_ang_z = std::atof(fields[3].c_str());
      age = 1;
      local_lin_x = global_lin_x;
      local_lin_y = global_lin_y;
      local_ang_z = global_ang_z;
    }
  }

  set_cmd(cmd, local_lin_x, local_lin_y, local_ang_z);
  udp.SetSend(cmd);
}

int main(void) {
  Custom custom;
  LoopFunc loop_control("control_loop", custom.dt, boost::bind(&Custom::RobotControl, &custom));
  LoopFunc loop_udpRecv("udp_recv", custom.dt, 3, boost::bind(&Custom::UDPRecv, &custom));
  LoopFunc loop_udpSend("udp_send", custom.dt, 3, boost::bind(&Custom::UDPSend, &custom));

  loop_udpSend.start();
  loop_udpRecv.start();
  loop_control.start();

  bool keep_running = true;
  while (keep_running) {
    if (std::cin.get() == 'q') {
      keep_running = false;
    }
  }

  return 0;
}
