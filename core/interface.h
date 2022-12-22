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

class Interface {
public:
    typedef bool (*accept_message_t)(uint16_t msgid, uint64_t &signature);
    typedef void (*handle_message_t)(const CanardRxTransfer& transfer);

    Interface(accept_message_t _accept_message, handle_message_t _handle_message) :
    accept_message(_accept_message),
    handle_message(_handle_message) {}

    // delete default constructor
    Interface() = delete;

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
    accept_message_t accept_message;
    handle_message_t handle_message;
};

} // namespace CubeFramework