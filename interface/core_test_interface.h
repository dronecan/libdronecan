#pragma once
#include <core/interface.h>

namespace CubeFramework {

class AbstractCoreTestInterface {
public:
    virtual void handle_transfer(CanardRxTransfer &transfer) = 0;
};

class CoreTestNetwork {
    template <int index>
    friend class CoreTestInterface;
public:
    static constexpr int N = 10;
    CoreTestNetwork() {}
    void route_msg(AbstractCoreTestInterface *send_iface, uint8_t source_node_id, uint8_t destination_node_id, Transfer transfer) {
        // prepare CanardRxTransfer
        CanardRxTransfer rx_transfer {};
        rx_transfer.data_type_id = transfer.data_type_id;
        rx_transfer.payload_len = transfer.payload_len; 
        rx_transfer.payload_head = (uint8_t*)transfer.payload;
        rx_transfer.transfer_id = *transfer.inout_transfer_id;
        rx_transfer.source_node_id = source_node_id;
        rx_transfer.transfer_type = transfer.transfer_type;
        // send to all interfaces
        for (auto iface : ifaces) {
            if (iface != send_iface && iface != nullptr) {
                iface->handle_transfer(rx_transfer);
            }
        }
    }
    CoreTestNetwork(const CoreTestNetwork&) = delete;
    CoreTestNetwork& operator=(const CoreTestNetwork&) = delete;

    static CoreTestNetwork& get_network() {
        static CoreTestNetwork network;
        return network;
    }
private:
    AbstractCoreTestInterface *ifaces[N];
};


template <int index>
class CoreTestInterface : public Interface<index>, public AbstractCoreTestInterface {
public:
    CoreTestInterface() {
        CoreTestNetwork::get_network().ifaces[index] = this;
    }

    /// @brief delete copy constructor and assignment operator
    CoreTestInterface(const CoreTestInterface&) = delete;
    CoreTestInterface& operator=(const CoreTestInterface&) = delete;

    /// @brief broadcast message to all listeners on Interface
    /// @param bc_transfer
    /// @return true if message was added to the queue
    bool broadcast(Transfer bcast_transfer) override {
        // call network router
        CoreTestNetwork::get_network().route_msg(this, node_id, 255, bcast_transfer);
        return true;
    }

    /// @brief request message from
    /// @param destination_node_id
    /// @param req_transfer
    /// @return true if request was added to the queue
    bool request(uint8_t destination_node_id, Transfer req_transfer) override {
        // call network router
        CoreTestNetwork::get_network().route_msg(this, node_id, destination_node_id, req_transfer);
        return true;
    }

    /// @brief respond to a request
    /// @param destination_node_id
    /// @param res_transfer
    /// @return true if response was added to the queue
    bool respond(uint8_t destination_node_id, Transfer res_transfer) override {
        // call network router
        CoreTestNetwork::get_network().route_msg(this, node_id, destination_node_id, res_transfer);
        return true;
    }

    /// @brief set node id
    /// @param node_id
    void set_node_id(uint8_t node_id) {
        this->node_id = node_id;
    }

    void handle_transfer(CanardRxTransfer &transfer) {
        uint64_t signature = 0;
        // check if message should be accepted
        if (this->accept_message(transfer.data_type_id, signature)) {
            // call message handler
            this->handle_message(transfer);
        }
    }

    /// @brief get singleton instance of interface, used by node to publish messages
    /// @return
    static CoreTestInterface<index>& get_singleton() {
        static CoreTestInterface<index> instance;
        return instance;
    }

private:
    uint8_t node_id;
};

} // namespace CubeFramework

/// @brief get singleton instance of interface, used by node to publish messages
#define CF_CORE_TEST_INTERFACE(index) CubeFramework::CoreTestInterface<index>::get_singleton()

