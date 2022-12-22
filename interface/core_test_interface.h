#pragma once
#include <core/interface.h>

namespace CubeFramework {

class CoreTestInterface;
class CoreTestNetwork {
    friend class CoreTestInterface;
public:
    static constexpr int N = 10;
    CoreTestNetwork() {}
    void route_msg(CoreTestInterface *send_iface, uint8_t source_node_id, uint8_t destination_node_id, Transfer transfer);
    CoreTestNetwork(const CoreTestNetwork&) = delete;
    CoreTestNetwork& operator=(const CoreTestNetwork&) = delete;

    static CoreTestNetwork& get_network() {
        static CoreTestNetwork network;
        return network;
    }
private:
    CoreTestInterface *ifaces[N];
};

class CoreTestInterface : public Interface {
public:
    CoreTestInterface(int index, accept_message_t _accept_message, handle_message_t _handle_message) :
    Interface(_accept_message, _handle_message) {
        CoreTestNetwork::get_network().ifaces[index] = this;
    }

    /// @brief delete copy constructor and assignment operator
    CoreTestInterface(const CoreTestInterface&) = delete;
    CoreTestInterface& operator=(const CoreTestInterface&) = delete;
    CoreTestInterface() = delete;

    /// @brief broadcast message to all listeners on Interface
    /// @param bc_transfer
    /// @return true if message was added to the queue
    bool broadcast(Transfer bcast_transfer) override;

    /// @brief request message from
    /// @param destination_node_id
    /// @param req_transfer
    /// @return true if request was added to the queue
    bool request(uint8_t destination_node_id, Transfer req_transfer) override;

    /// @brief respond to a request
    /// @param destination_node_id
    /// @param res_transfer
    /// @return true if response was added to the queue
    bool respond(uint8_t destination_node_id, Transfer res_transfer) override;

    /// @brief set node id
    /// @param node_id
    void set_node_id(uint8_t node_id);

    void handle_transfer(CanardRxTransfer &transfer);

private:
    uint8_t node_id;
};

} // namespace CubeFramework

/// @brief get singleton instance of interface, used by node to publish messages
#define CF_CORE_TEST_INTERFACE_DEFINE(index) \
    CubeFramework::CoreTestInterface test_interface_##index \
    {index, CubeFramework::HandlerList<index>::accept_message, CubeFramework::HandlerList<index>::handle_message};

#define CF_CORE_TEST_INTERFACE(index) test_interface_##index
