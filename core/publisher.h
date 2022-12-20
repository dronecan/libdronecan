#pragma once
#include "node.h"
#include <canard.h>

namespace CubeFramework {

template <typename msgtype, int index>
class Publisher {
public:
    Publisher(Interface<index> &_iface) :
    iface(_iface) {}

    // delete copy constructor and assignment operator
    Publisher(const Publisher&) = delete;

    /// @brief Broadcast a message
    /// @param msg message to send
    /// @return true if the message was put into the queue successfully
    bool broadcast(typename msgtype::c_msg_type& msg) {
        // encode the message
        uint16_t len = msgtype::encode(&msg, msg_buf);
        // send the message if encoded successfully
        if (len > 0) {
            Transfer msg_transfer;
            msg_transfer.transfer_type = CanardTransferTypeBroadcast;
            msg_transfer.data_type_id = msgtype::ID;
            msg_transfer.data_type_signature = msgtype::SIGNATURE;
            msg_transfer.payload = msg_buf;
            msg_transfer.payload_len = len;
            msg_transfer.priority = priority;
#if CANARD_ENABLE_CANFD
            msg_transfer.canfd = canfd;
#endif
#if CANARD_MULTI_IFACE
            msg_transfer.iface_mask = iface_mask;
#endif
            msg_transfer.inout_transfer_id = &inout_transfer_id;
            return iface.broadcast(msg_transfer);
        }
        return false;
    }
private:
    Interface<index> &iface;
    uint8_t msg_buf[msgtype::MAX_SIZE];
    uint8_t inout_transfer_id = 0;
    uint8_t priority = CANARD_TRANSFER_PRIORITY_MEDIUM;
#if CANARD_MULTI_IFACE
    uint8_t iface_mask = CANARD_IFACE_ALL;
#endif
#if CANARD_ENABLE_CANFD
    bool canfd = false;
#endif
};
} // namespace CubeFramework

/// @brief Macro to create a publisher
/// @param PUBNAME name of the publisher
/// @param MSGTYPE type of the message
/// @param IFACETYPE name of the interface
#define CF_PUBLISHER(PUBNAME, IFACETYPE, MSGTYPE) \
    CubeFramework::Publisher<MSGTYPE##_cxx_iface, 0> PUBNAME{IFACETYPE<0>::get_singleton()};

/// @brief Macro to create a publisher with an interface index
/// @param ID index of the interface
/// @param PUBNAME name of the publisher
/// @param MSGTYPE type of the message
/// @param IFACETYPE type of the interface
#define CF_PUBLISHER_INDEX(ID, PUBNAME, IFACETYPE, MSGTYPE) \
    CubeFramework::Publisher<MSGTYPE##_cxx_iface, ID> PUBNAME{IFACETYPE<ID>::get_singleton()};
