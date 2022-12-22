#include "common_test.h"
#include <interface/canard_interface.h>
#include <core/publisher.h>
#include <core/subscriber.h>
#include <dronecan_msgs.h>
#include <gtest/gtest.h>
#include <core/service_server.h>
#include <core/service_client.h>
#include <time.h>

using namespace CubeFramework;
namespace StaticCanardTest {

CanardRxState rx_state  {
        .next = NULL,
        .buffer_blocks = NULL,
        .dtid_tt_snid_dnid = 0
    };

static const int test_header_size = CANARD_MULTIFRAME_RX_PAYLOAD_HEAD_SIZE;

///////////// TESTS for Subscriber and Publisher //////////////
static bool called_handle_node_status = false;
static uavcan_protocol_NodeStatus sent_msg;
static CanardRxTransfer last_transfer;
void handle_node_status(const CanardRxTransfer &transfer, const uavcan_protocol_NodeStatus &msg) {
    called_handle_node_status = true;
    last_transfer = transfer;
    // check if message is correct
    ASSERT_EQ(memcmp(&msg, &sent_msg, sizeof(uavcan_protocol_NodeStatus)), 0);
}

TEST(StaticCanardTest, test_publish_subscribe) {
    CF_CANARD_TEST_INTERFACE_DEFINE(0);
    CF_CANARD_TEST_INTERFACE_DEFINE(1);
    // create publisher for message uavcan_protocol_NodeStatus on interface CoreTestInterface
    // with callback function handle_node_status
    CF_PUBLISHER(CF_CANARD_TEST_INTERFACE(0), node_status_pub0, uavcan_protocol_NodeStatus);
    // create publisher for message uavcan_protocol_NodeStatus on different interface instance CoreTestInterface
    // with callback function handle_node_status
    CF_PUBLISHER(CF_CANARD_TEST_INTERFACE(1), node_status_pub1, uavcan_protocol_NodeStatus);

    // create subscriber for message uavcan_protocol_NodeStatus on interface CoreTestInterface
    CF_SUBSCRIBE_MSG(node_status_sub0, uavcan_protocol_NodeStatus, handle_node_status);
    // create subscriber for message uavcan_protocol_NodeStatus on different interface instance CoreTestInterface
    // with callback function handle_node_status
    CF_SUBSCRIBE_MSG_INDEXED(1, node_status_sub1, uavcan_protocol_NodeStatus, handle_node_status);

    uint8_t buffer0[1024] {};
    uint8_t buffer1[1024] {};
    CF_CANARD_TEST_INTERFACE(0).init(buffer0, sizeof(buffer0));
    CF_CANARD_TEST_INTERFACE(1).init(buffer1, sizeof(buffer1));

    // set node id for interfaces
    CF_CANARD_TEST_INTERFACE(0).set_node_id(1);
    CF_CANARD_TEST_INTERFACE(1).set_node_id(2);

    // create message
    uavcan_protocol_NodeStatus msg {};
    msg.uptime_sec = 1;
    msg.health = 2;
    msg.mode = 3;
    msg.sub_mode = 4;
    msg.vendor_specific_status_code = 5;
    sent_msg = msg;
    timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    uint64_t timestamp = ts.tv_sec * 1000000 + ts.tv_nsec / 1000;

    called_handle_node_status = false;
    // publish message
    ASSERT_TRUE(node_status_pub0.broadcast(msg));
    CF_CANARD_TEST_INTERFACE(0).update_tx(timestamp);

    // check if message was received
    ASSERT_TRUE(called_handle_node_status);
}

} // namespace StaticCanardTest