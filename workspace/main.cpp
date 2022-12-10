/* mros2 example
 * Copyright (c) 2022 mROS-base
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "mbed.h"
#include "mros2.h"
#include "std_msgs/msg/string.hpp"
// #include "EthernetInterface.h"

#include "WIZnetInterface.h"
NetworkInterface *net;
static WIZnetInterface eth1(D11, D12, D13, D6, D3);
static uint8_t WIZNET_DEFAULT_TESTMAC[6] = {0x00, 0x08, 0xdc,
                                            0x19, 0x85, 0xa8};

#define IP_ADDRESS ("192.168.11.2") /* IP address */
#define SUBNET_MASK ("255.255.255.0") /* Subnet mask */
#define DEFAULT_GATEWAY ("192.168.11.1") /* Default gateway */
#define DNS_SERVER ("8.8.8.8") /* DNS server */

/* convert TARGET_NAME to put into message */
#define quote(x) std::string(q(x))
#define q(x) #x


void userCallback(std_msgs::msg::String *msg)
{
  printf("subscribed msg: '%s'\r\n", msg->data.c_str());
}

nsapi_size_or_error_t wiznet_init()
{
  eth1.init(WIZNET_DEFAULT_TESTMAC, IP_ADDRESS, SUBNET_MASK, DEFAULT_GATEWAY);
  eth1.setDnsServerIP(DNS_SERVER);
  net = &eth1;

  return net->connect();
}

int main() {
  // serial begin
  static BufferedSerial serial_port(USBTX, USBRX);
  serial_port.set_baud(115200);
  // EthernetInterface network;
  // network.set_dhcp(false);
  // network.set_network(IP_ADDRESS, SUBNET_MASK, DEFAULT_GATEWAY);
  // nsapi_size_or_error_t result = network.connect();
  nsapi_size_or_error_t result = wiznet_init();

  printf("mbed mros2 start!\r\n");
  printf("app name: echoback_string\r\n");
  mros2::init(0, NULL);
  MROS2_DEBUG("mROS 2 initialization is completed\r\n");

  mros2::Node node = mros2::Node::create_node("mros2_node");
  mros2::Publisher pub = node.create_publisher<std_msgs::msg::String>("to_linux", 10);
  mros2::Subscriber sub = node.create_subscription<std_msgs::msg::String>("to_stm", 10, userCallback);

  osDelay(100);
  MROS2_INFO("ready to pub/sub message\r\n");

  auto count = 0;
  while (1) {
    auto msg = std_msgs::msg::String();
    msg.data = "Hello from mros2-mbed onto " + quote(TARGET_NAME) + ": " + std::to_string(count++);
    printf("publishing msg: '%s'\r\n", msg.data.c_str());
    pub.publish(msg);
    osDelay(1000);
  }

  mros2::spin();
  return 0;
}