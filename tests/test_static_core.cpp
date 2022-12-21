#include <interface/core_test_interface.h>
#include <core/publisher.h>
#include <core/subscriber.h>
#define DRONECAN_CXX_WRAPPERS
#include <dronecan_msgs.h>
#include <gtest/gtest.h>
#include <core/service_server.h>
#include <core/service_client.h>

using namespace CubeFramework;

INITIALISE_NODE(CoreTestInterface, 0); // initialise node 0
INITIALISE_NODE(CoreTestInterface, 1); // initialise node 1

// test publisher and subscriber
static bool called_handle_node_status = false;
static uavcan_protocol_NodeStatus sent_msg;
static CanardRxTransfer last_transfer;
void handle_node_status(const CanardRxTransfer &transfer, const uavcan_protocol_NodeStatus &msg) {
    called_handle_node_status = true;
    last_transfer = transfer;
    // check if message is correct
    ASSERT_EQ(memcmp(&msg, &sent_msg, sizeof(uavcan_protocol_NodeStatus)), 0);
}

TEST(StaticCoreTest, test_publish_subscribe) {
    // create publisher for message uavcan_protocol_NodeStatus on interface CoreTestInterface
    // with callback function handle_node_status
    CF_PUBLISHER(node_status_pub0, CoreTestInterface, uavcan_protocol_NodeStatus);
    // create publisher for message uavcan_protocol_NodeStatus on different interface instance CoreTestInterface
    // with callback function handle_node_status
    CF_PUBLISHER_INDEX(1, node_status_pub1, CoreTestInterface, uavcan_protocol_NodeStatus);

    // create subscriber for message uavcan_protocol_NodeStatus on interface CoreTestInterface
    CF_SUBSCRIBE_MSG(node_status_sub0, uavcan_protocol_NodeStatus, handle_node_status);
    // create subscriber for message uavcan_protocol_NodeStatus on different interface instance CoreTestInterface
    // with callback function handle_node_status
    CF_SUBSCRIBE_MSG_INDEXED(1, node_status_sub1, uavcan_protocol_NodeStatus, handle_node_status);

    // set node id for interfaces
    CF_CORE_TEST_INTERFACE(0).set_node_id(1);
    CF_CORE_TEST_INTERFACE(1).set_node_id(2);

    // create message
    uavcan_protocol_NodeStatus msg {};
    msg.uptime_sec = 1;
    msg.health = 2;
    msg.mode = 3;
    msg.sub_mode = 4;
    msg.vendor_specific_status_code = 5;
    sent_msg = msg;

    called_handle_node_status = false;
    // publish message
    ASSERT_TRUE(node_status_pub0.broadcast(msg));
    // check if message was received
    ASSERT_TRUE(called_handle_node_status);
}

// test multiple subscribers

// default test subscriber without
class TestSubscriber0 {
public:
    TestSubscriber0() {
        CF_BIND(node_status_sub, this);
    }
    void handle_node_status(const CanardRxTransfer &transfer, const uavcan_protocol_NodeStatus &msg);
    static int call_counts;

private:
    CF_SUBSCRIBE_MSG_CLASS(node_status_sub, uavcan_protocol_NodeStatus, TestSubscriber0, &TestSubscriber0::handle_node_status);
};

void TestSubscriber0::handle_node_status(const CanardRxTransfer &transfer, const uavcan_protocol_NodeStatus &msg) {
    called_handle_node_status = true;
    last_transfer = transfer;
    // check if message is correct
    ASSERT_EQ(memcmp(&msg, &sent_msg, sizeof(uavcan_protocol_NodeStatus)), 0);
    call_counts++;
}

int TestSubscriber0::call_counts = 0;

// test subsriber with index 1
class TestSubscriber1 {
public:
    TestSubscriber1() {
        CF_BIND(node_status_sub, this);
    }
    void handle_node_status(const CanardRxTransfer &transfer, const uavcan_protocol_NodeStatus &msg) {
        called_handle_node_status = true;
        last_transfer = transfer;
        // check if message is correct
        ASSERT_EQ(memcmp(&msg, &sent_msg, sizeof(uavcan_protocol_NodeStatus)), 0);
        call_counts++;
    }
    static int call_counts;
private:
    CF_SUBSCRIBE_MSG_CLASS_INDEX(1, node_status_sub, uavcan_protocol_NodeStatus, TestSubscriber1, &TestSubscriber1::handle_node_status);
};

int TestSubscriber1::call_counts = 0;

// Create 5 subscribers and check if all of them are called
TEST(StaticCoreTest, test_multiple_subscribers) {
    // create publisher for message uavcan_protocol_NodeStatus on interface CoreTestInterface
    // with callback function handle_node_status
    CF_PUBLISHER(node_status_pub0, CoreTestInterface, uavcan_protocol_NodeStatus);
    // create publisher for message uavcan_protocol_NodeStatus on different interface instance CoreTestInterface
    // with callback function handle_node_status
    CF_PUBLISHER_INDEX(1, node_status_pub1, CoreTestInterface, uavcan_protocol_NodeStatus);

    TestSubscriber0 test_subscriber0[5] __attribute__((unused)) {};
    TestSubscriber1 test_subscriber1[5] __attribute__((unused)) {};

    // set node id for interfaces
    CF_CORE_TEST_INTERFACE(0).set_node_id(1);
    CF_CORE_TEST_INTERFACE(1).set_node_id(2);

    // create message
    uavcan_protocol_NodeStatus msg {};
    msg.uptime_sec = 1;
    msg.health = 2;
    msg.mode = 3;
    msg.sub_mode = 4;
    msg.vendor_specific_status_code = 5;
    sent_msg = msg;

    // publish message
    ASSERT_TRUE(node_status_pub0.broadcast(msg));
    // check if message was received 5 times
    ASSERT_EQ(TestSubscriber1::call_counts, 5);
    // publish message
    ASSERT_TRUE(node_status_pub1.broadcast(msg));
    // check if message was received 5 times
    ASSERT_EQ(TestSubscriber0::call_counts, 5);
}

// test service server
bool handle_get_node_info_response_called = false;
void handle_get_node_info_response(const CanardRxTransfer &transfer, const uavcan_protocol_GetNodeInfoResponse &res) {
    ASSERT_EQ(res.status.uptime_sec, 1);
    ASSERT_EQ(res.status.health, 2);
    ASSERT_EQ(res.status.mode, 3);
    ASSERT_EQ(res.status.sub_mode, 4);
    ASSERT_EQ(res.status.vendor_specific_status_code, 5);
    ASSERT_EQ(res.software_version.major, 1);
    ASSERT_EQ(res.software_version.minor, 2);
    ASSERT_EQ(res.hardware_version.major, 3);
    ASSERT_EQ(res.hardware_version.minor, 4);
    ASSERT_EQ(res.name.len, strlen("helloworld"));
    ASSERT_EQ(memcmp(res.name.data, "helloworld", res.name.len), 0);
    handle_get_node_info_response_called = true;
}

void handle_get_node_info_request0(const CanardRxTransfer &transfer, const uavcan_protocol_GetNodeInfoRequest &req);
CF_CREATE_SERVER(get_node_info_server0, uavcan_protocol_GetNodeInfo, handle_get_node_info_request0);
void handle_get_node_info_request1(const CanardRxTransfer &transfer, const uavcan_protocol_GetNodeInfoRequest &req);
CF_CREATE_SERVER_INDEX(1, get_node_info_server1, uavcan_protocol_GetNodeInfo, handle_get_node_info_request1);

void handle_get_node_info_request0(const CanardRxTransfer &transfer, const uavcan_protocol_GetNodeInfoRequest &req) {
    uavcan_protocol_GetNodeInfoResponse res {};
    res.status.uptime_sec = 1;
    res.status.health = 2;
    res.status.mode = 3;
    res.status.sub_mode = 4;
    res.status.vendor_specific_status_code = 5;
    res.software_version.major = 1;
    res.software_version.minor = 2;
    res.hardware_version.major = 3;
    res.hardware_version.minor = 4;
    res.name.len = strlen("helloworld");
    memcpy(res.name.data, "helloworld", res.name.len);
    get_node_info_server0.respond(transfer, res);
}

void handle_get_node_info_request1(const CanardRxTransfer &transfer, const uavcan_protocol_GetNodeInfoRequest &req) {
    uavcan_protocol_GetNodeInfoResponse res {};
    res.status.uptime_sec = 1;
    res.status.health = 2;
    res.status.mode = 3;
    res.status.sub_mode = 4;
    res.status.vendor_specific_status_code = 5;
    res.software_version.major = 1;
    res.software_version.minor = 2;
    res.hardware_version.major = 3;
    res.hardware_version.minor = 4;
    res.name.len = strlen("helloworld");
    memcpy(res.name.data, "helloworld", res.name.len);
    get_node_info_server1.respond(transfer, res);
}

TEST(StaticCoreTest, test_service) {
    // create client for service uavcan_protocol_GetNodeInfo on interface CoreTestInterface
    // with response callback function handle_get_node_info_response
    CF_CREATE_CLIENT(get_node_info_client0, uavcan_protocol_GetNodeInfo, handle_get_node_info_response);
    // create client for service uavcan_protocol_GetNodeInfo on different interface instance CoreTestInterface
    // with response callback function handle_get_node_info_response
    CF_CREATE_CLIENT_INDEX(1, get_node_info_client1, uavcan_protocol_GetNodeInfo, handle_get_node_info_response);

    // set node id for interfaces
    CF_CORE_TEST_INTERFACE(0).set_node_id(1);
    CF_CORE_TEST_INTERFACE(1).set_node_id(2);

    // create request
    uavcan_protocol_GetNodeInfoRequest req {};
    handle_get_node_info_response_called = false;
    // send request
    ASSERT_TRUE(get_node_info_client0.request(2, req));
    // check if response was received
    ASSERT_TRUE(handle_get_node_info_response_called);

    handle_get_node_info_response_called = false;
    // send request
    ASSERT_TRUE(get_node_info_client1.request(1, req));
    // check if response was received
    ASSERT_TRUE(handle_get_node_info_response_called);
}
