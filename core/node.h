#pragma once

#include <stdint.h>
#include <canard.h>

namespace CubeFramework {
 
/// @brief Base class for all message handlers.
class Handler {
public:
    virtual void handle_message(const CanardRxTransfer& transfer) = 0;
};

/// @brief Node to register all handled message types.
/// @tparam index Index of the node.
template <int index>
class Node {
public:
    /// @brief Constructor call creates a new handler entry for the message type.
    Node(CanardTransferType _transfer_type, uint16_t _msgid, uint64_t _signature) {
        // find the entry in the registry with the same msgid
        next = head;
        head = this;
        msgid = _msgid;
        signature = _signature;
        transfer_type = _transfer_type;
    }

    /// @brief delete copy constructor and assignment operator
    Node(const Node&) = delete;

    /// @brief should accept message id
    /// @param msgid
    /// @return true if message id is accepted and also sets the signature
    static bool accept_message(uint16_t msgid, uint64_t &signature)
    {
        Node* entry = head;
        while (entry != nullptr) {
            if (entry->msgid == msgid && entry->branch_head != nullptr) {
                signature = entry->signature;
                return true;
            }
            entry = entry->next;
        }
        return false;
    }

    /// @brief parse the message and call the callbacks
    /// @param buf
    /// @param transfer_id
    static void handle_message(const CanardRxTransfer& transfer)
    {
        Node* entry = head;
        while (entry != nullptr) {
            if (transfer.data_type_id == entry->msgid &&
                entry->branch_head != nullptr &&
                entry->transfer_type == transfer.transfer_type) {
                entry->branch_head->handle_message(transfer);
            }
            entry = entry->next;
        }
    }

    Handler* get_branch_head() {
        return branch_head;
    }

    void set_branch_head(Handler* _branch_head) {
        branch_head = _branch_head;
    }

private:
    Node* next;
    static Node* head;

    uint16_t msgid;
    uint64_t signature;
    Handler* branch_head = nullptr; // head of the list of message handlers
    CanardTransferType transfer_type;
};

template <int index>
Node<index>* Node<index>::head = nullptr;

} // namespace CubeFramework
