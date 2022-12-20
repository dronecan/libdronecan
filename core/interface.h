#pragma once
#include <stdint.h>
#include <canard.h>

namespace CubeFramework {

struct Transfer {
    CanardTransferType transfer_type;
    uint64_t data_type_signature;
    uint16_t data_type_id;
    uint8_t* inout_transfer_id;
    uint8_t priority;
    const void* payload;
    uint16_t payload_len;
    uint8_t iface_mask;
    bool canfd;
};

class AbstractInterface {
protected:
    /// @brief check if the message should be accepted
    /// @param transfer
    virtual bool accept_message(uint16_t msgid, uint64_t &signature) = 0;

    /// @brief process a received message through callbacks
    /// @param transfer
    virtual void handle_message(const CanardRxTransfer& transfer) = 0;
};

template <int index>
class Node;

template <int index>
class Interface : public AbstractInterface {
public:
    /// @brief broadcast message to all listeners on Interface
    /// @param bc_transfer
    /// @return true if message was added to the queue
    virtual bool broadcast(Transfer bcast_transfer) = 0;

    /// @brief request message from
    /// @param destination_node_id
    /// @param req_transfer
    /// @return true if request was added to the queue
    virtual bool request(uint8_t destination_node_id, Transfer req_transfer) = 0;

    /// @brief respond to a request
    /// @param destination_node_id
    /// @param res_transfer
    /// @return true if response was added to the queue
    virtual bool respond(uint8_t destination_node_id, Transfer res_transfer) = 0;

protected:
    /// @brief check if the message should be accepted
    /// @param transfer
    bool accept_message(uint16_t msgid, uint64_t &signature) override
    {
        return Node<index>::accept_message(msgid, signature);
    }

    /// @brief process a received message through callbacks
    /// @param transfer
    void handle_message(const CanardRxTransfer& transfer) override
    {
        Node<index>::handle_message(transfer);
    }
};

} // namespace CubeFramework