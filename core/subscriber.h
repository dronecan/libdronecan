#pragma once

#include <stdint.h>
#include <canard.h>
#include "callbacks.h"
#include "handler_list.h"

namespace CubeFramework {

/// @brief Class to handle broadcast messages
/// @tparam msgtype 
/// @tparam handlerlist
template <typename msgtype, typename handlerlist>
class Subscriber : public Handler {
public:
    /// @brief Constructor call creates a new entry of listener into the callback list for msgtype.
    /// @param cb 
    Subscriber(typename CallbackContainer<typename msgtype::c_msg_type>::Callback &_cb) :
    cb (_cb) {
        static handlerlist _handler_list(CanardTransferTypeBroadcast, msgtype::ID, msgtype::SIGNATURE);
        handler_list = &_handler_list;
        Subscriber<msgtype, handlerlist>* head = (Subscriber<msgtype, handlerlist>*)handler_list->get_branch_head();
        next = head;
        handler_list->set_branch_head(this);
    }

    // delete copy constructor and assignment operator
    Subscriber(const Subscriber&) = delete;

    // destructor, remove the entry from the singly-linked list
    ~Subscriber() {
        Subscriber<msgtype, handlerlist>* entry = this;
        Subscriber<msgtype, handlerlist>* prev = nullptr;
        while (entry != nullptr) {
            if (entry == this) {
                if (prev != nullptr) {
                    prev->next = entry->next;
                } else {
                    handler_list->set_branch_head(entry->next);
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
        Subscriber<msgtype, handlerlist>* entry = this;
        while (entry != nullptr) {
            entry->cb(transfer, msg);
            entry = entry->next;
        }
    }

private:
    Subscriber<msgtype, handlerlist>* next;
    handlerlist *handler_list;
    typename CallbackContainer<typename msgtype::c_msg_type>::Callback &cb;
};
} // namespace CubeFramework

/// Helper macros to register message handlers

/// @brief Register a message handler using indexed handler_list.
/// @param NID handler_list instance id
/// @param SUBNAME name of the subscriber instance
/// @param MSGTYPE message type name
/// @param MSG_HANDLER callback function, called when message is received
#define CF_SUBSCRIBE_MSG_INDEXED(NID, SUBNAME, MSGTYPE, MSG_HANDLER) \
    CubeFramework::CallbackContainer<MSGTYPE##_cxx_iface::c_msg_type>::StaticCallback SUBNAME##_callback{MSG_HANDLER}; \
    CubeFramework::Subscriber<MSGTYPE##_cxx_iface, CubeFramework::HandlerList<NID>> SUBNAME{SUBNAME##_callback};

/// @brief Register a message handler
/// @param SUBNAME name of the subscriber instance
/// @param MSGTYPE message type name
/// @param MSG_HANDLER callback function, called when message is received
#define CF_SUBSCRIBE_MSG(SUBNAME, MSGTYPE, MSG_HANDLER) \
    CubeFramework::CallbackContainer<MSGTYPE##_cxx_iface::c_msg_type>::StaticCallback SUBNAME##_callback{MSG_HANDLER}; \
    CubeFramework::Subscriber<MSGTYPE##_cxx_iface, CubeFramework::HandlerList<0>> SUBNAME{SUBNAME##_callback};

/// @brief Register a message handler with object instance using indexed handler_list
/// @param NID handler_list instance id
/// @param SUBNAME name of the subscriber instance
/// @param MSGTYPE message type name
/// @param CLASS class name
/// @param MSG_HANDLER callback function, called when message is received
#define CF_SUBSCRIBE_MSG_CLASS_INDEX(NID, SUBNAME, MSGTYPE, CLASS, MSG_HANDLER) \
    CubeFramework::CallbackContainer<MSGTYPE##_cxx_iface::c_msg_type>::ObjCallback<CLASS> SUBNAME##_callback{MSG_HANDLER}; \
    CubeFramework::Subscriber<MSGTYPE##_cxx_iface, CubeFramework::HandlerList<NID>> SUBNAME{SUBNAME##_callback};

/// @brief Register a message handler with object instance
/// @param SUBNAME name of the subscriber instance
/// @param MSGTYPE message type name
/// @param CLASS class name
/// @param MSG_HANDLER callback function, called when message is received
#define CF_SUBSCRIBE_MSG_CLASS(SUBNAME, MSGTYPE, CLASS, MSG_HANDLER) \
    CubeFramework::CallbackContainer<MSGTYPE##_cxx_iface::c_msg_type>::ObjCallback<CLASS> SUBNAME##_callback{MSG_HANDLER}; \
    CubeFramework::Subscriber<MSGTYPE##_cxx_iface, CubeFramework::HandlerList<0>> SUBNAME{SUBNAME##_callback};
