#pragma once

#include <stdint.h>
#include <canard.h>
#include "callbacks.h"
#include "node.h"

namespace CubeFramework {

/// @brief Class to handle broadcast messages
/// @tparam msgtype 
/// @tparam msgnode
template <typename msgtype, typename msgnode>
class Subscriber : public Handler {
public:
    /// @brief Constructor call creates a new entry of listener into the callback list for msgtype.
    /// @param cb 
    Subscriber(typename CallbackContainer<typename msgtype::c_msg_type>::Callback &_cb) :
    cb (_cb) {
        static msgnode _node(CanardTransferTypeBroadcast, msgtype::ID, msgtype::SIGNATURE);
        node = &_node;
        Subscriber<msgtype, msgnode>* head = (Subscriber<msgtype, msgnode>*)node->get_branch_head();
        next = head;
        node->set_branch_head(this);
    }

    // delete copy constructor and assignment operator
    Subscriber(const Subscriber&) = delete;

    // destructor, remove the entry from the singly-linked list
    ~Subscriber() {
        Subscriber<msgtype, msgnode>* entry = this;
        Subscriber<msgtype, msgnode>* prev = nullptr;
        while (entry != nullptr) {
            if (entry == this) {
                if (prev != nullptr) {
                    prev->next = entry->next;
                } else {
                    node->set_branch_head(entry->next);
                }
                break;
            }
            prev = entry;
            entry = entry->next;
        }
    }

    /// @brief parse the message and call the callback
    /// @param buf
    /// @param buflen
    /// @param transfer
    void handle_message(const CanardRxTransfer& transfer) override {
        typename msgtype::c_msg_type msg {};
        msgtype::decode(&transfer, &msg);
        // call all registered callbacks
        Subscriber<msgtype, msgnode>* entry = this;
        while (entry != nullptr) {
            entry->cb(transfer, msg);
            entry = entry->next;
        }
    }

private:
    Subscriber<msgtype, msgnode>* next;
    msgnode *node;
    typename CallbackContainer<typename msgtype::c_msg_type>::Callback &cb;
};
} // namespace CubeFramework

/// Helper macros to register message handlers

/// @brief Register a message handler using indexed node.
/// @param NID node instance id
/// @param SUBNAME name of the subscriber instance
/// @param MSGTYPE message type name
/// @param MSG_HANDLER callback function, called when message is received
#define CF_SUBSCRIBE_MSG_INDEXED(NID, SUBNAME, MSGTYPE, MSG_HANDLER) \
    CubeFramework::CallbackContainer<MSGTYPE##_cxx_iface::c_msg_type>::StaticCallback SUBNAME##_callback{MSG_HANDLER}; \
    CubeFramework::Subscriber<MSGTYPE##_cxx_iface, CubeFramework::Node<NID>> SUBNAME{SUBNAME##_callback};

/// @brief Register a message handler
/// @param SUBNAME name of the subscriber instance
/// @param MSGTYPE message type name
/// @param MSG_HANDLER callback function, called when message is received
#define CF_SUBSCRIBE_MSG(SUBNAME, MSGTYPE, MSG_HANDLER) \
    CubeFramework::CallbackContainer<MSGTYPE##_cxx_iface::c_msg_type>::StaticCallback SUBNAME##_callback{MSG_HANDLER}; \
    CubeFramework::Subscriber<MSGTYPE##_cxx_iface, CubeFramework::Node<0>> SUBNAME{SUBNAME##_callback};

/// @brief Register a message handler with object instance using indexed node
/// @param NID node instance id
/// @param SUBNAME name of the subscriber instance
/// @param MSGTYPE message type name
/// @param OBJ object instance
/// @param MSG_HANDLER callback function, called when message is received
#define CF_SUBSCRIBE_MSG_CLASS_INDEX(NID, SUBNAME, MSGTYPE, CLASS, MSG_HANDLER) \
    CubeFramework::CallbackContainer<MSGTYPE##_cxx_iface::c_msg_type>::ObjCallback<CLASS> SUBNAME##_callback{MSG_HANDLER}; \
    CubeFramework::Subscriber<MSGTYPE##_cxx_iface, CubeFramework::Node<NID>> SUBNAME{SUBNAME##_callback};

/// @brief Register a message handler with object instance
/// @param SUBNAME name of the subscriber instance
/// @param MSGTYPE message type name
/// @param OBJ object instance
/// @param MSG_HANDLER callback function, called when message is received
#define CF_SUBSCRIBE_MSG_CLASS(SUBNAME, MSGTYPE, CLASS, MSG_HANDLER) \
    CubeFramework::CallbackContainer<MSGTYPE##_cxx_iface::c_msg_type>::ObjCallback<CLASS> SUBNAME##_callback{MSG_HANDLER}; \
    CubeFramework::Subscriber<MSGTYPE##_cxx_iface, CubeFramework::Node<0>> SUBNAME{SUBNAME##_callback};
