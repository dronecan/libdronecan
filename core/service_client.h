#pragma once
#include "node.h"
#include "interface.h"

namespace CubeFramework {
template <typename svctype, typename svcnode>
class Client : public Handler {
    typedef struct transfer_id_s {
        uint8_t server_node_id;
        uint8_t transfer_id;
        transfer_id_s* next;
    } transfer_id_t;
    static transfer_id_t* head;
    uint8_t server_node_id;
    uint8_t transfer_id;
public:
    Client(Interface &_interface, typename CallbackContainer<typename svctype::c_rsp_type>::Callback &_cb) : 
    interface(_interface),
    cb(_cb) {
        static svcnode _node(CanardTransferTypeResponse, svctype::ID, svctype::SIGNATURE);
        node = &_node;
        Client<svctype, svcnode>* head = (Client<svctype, svcnode>*)node->get_branch_head();
        next = head;
        node->set_branch_head(this);
    }

    // delete copy constructor and assignment operator
    Client(const Client&) = delete;

    // destructor, remove the entry from the singly-linked list
    ~Client() {
        Client<svctype, svcnode>* entry = this;
        Client<svctype, svcnode>* prev = nullptr;
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

    void handle_message(const CanardRxTransfer& transfer) {
        typename svctype::c_rsp_type msg {};
        svctype::rsp_decode(&transfer, &msg);

        // scan through the list of entries for corresponding server node id and transfer id
        Client<svctype, svcnode>* entry = this;
        while (entry != nullptr) {
            if (entry->server_node_id == transfer.source_node_id
                && entry->transfer_id == transfer.transfer_id) {
                entry->cb(transfer, msg);
                return;
            }
            entry = entry->next;
        }
    }

    bool request(uint8_t destination_node_id, typename svctype::c_req_type& msg) {
        // encode the message
        uint16_t len = svctype::req_encode(&msg, req_buf);
        // send the message if encoded successfully
        req_transfer.transfer_type = CanardTransferTypeRequest;
        req_transfer.data_type_id = svctype::ID;
        req_transfer.data_type_signature = svctype::SIGNATURE;
        req_transfer.inout_transfer_id = get_transfer_id(destination_node_id);
        req_transfer.payload = req_buf;
        req_transfer.payload_len = len;
        req_transfer.priority = priority;
#if CANARD_ENABLE_CANFD
        req_transfer.canfd = canfd;
#endif
#if CANARD_MULTI_IFACE
        req_transfer.iface_mask = iface_mask;
#endif
        server_node_id = destination_node_id;
        transfer_id = *req_transfer.inout_transfer_id;
        return interface.request(destination_node_id, req_transfer);
    }

private:
    Interface &interface;

    uint8_t* get_transfer_id(uint8_t server_node_id) {
        transfer_id_t* entry = head;
        while (entry != nullptr) {
            if (entry->server_node_id == server_node_id) {
                return &entry->transfer_id;
            }
            entry = entry->next;
        }
        if (entry == nullptr) {
            entry = new transfer_id_t;
            if (entry == nullptr) {
                return nullptr;
            }
            entry->server_node_id = server_node_id;
            entry->transfer_id = 0;
            entry->next = head;
            head = entry;
        }
        return &entry->transfer_id;
    }

    Transfer req_transfer;
    uint8_t req_buf[svctype::REQ_MAX_SIZE];
    typename CallbackContainer<typename svctype::c_rsp_type>::Callback &cb;
    svcnode *node;

    uint8_t priority = CANARD_TRANSFER_PRIORITY_MEDIUM;
#if CANARD_MULTI_IFACE
    uint8_t iface_mask = CANARD_IFACE_ALL;
#endif
#if CANARD_ENABLE_CANFD
    bool canfd = false;
#endif
    Client<svctype, svcnode>* next;
};

template <typename svctype, typename svcnode>
typename Client<svctype, svcnode>::transfer_id_t *Client<svctype, svcnode>::head = nullptr;

} // namespace CubeFramework

/// Helper macros to create client instances

/// @brief create a client instance
/// @param IFACE interface instance name
/// @param CLINAME client instance name
/// @param SVCTYPE service type
/// @param RSPHANDLER response handler callback
#define CF_CREATE_CLIENT(IFACE, CLINAME, SVCTYPE, RSPHANDLER) \
    CubeFramework::CallbackContainer<SVCTYPE##_cxx_iface::c_rsp_type>::StaticCallback CLINAME##_callback{RSPHANDLER}; \
    CubeFramework::Client<SVCTYPE##_cxx_iface, CubeFramework::Node<0>> CLINAME{IFACE, CLINAME##_callback};


/// @brief create a client instance with indexed Node
/// @param IFACE interface instance name
/// @param ID Node index
/// @param CLINAME client instance name
/// @param SVCTYPE service type
/// @param RSPHANDLER response handler callback
#define CF_CREATE_CLIENT_INDEX(IFACE, ID, CLINAME, SVCTYPE, RSPHANDLER) \
    CubeFramework::CallbackContainer<SVCTYPE##_cxx_iface::c_rsp_type>::StaticCallback CLINAME##_callback{RSPHANDLER}; \
    CubeFramework::Client<SVCTYPE##_cxx_iface, CubeFramework::Node<ID>> CLINAME{IFACE, CLINAME##_callback};

/// @brief create a client instance
/// @param IFACE interface instance name
/// @param CLINAME client instance name
/// @param SVCTYPE service type
/// @param CLASS class name
/// @param RSPHANDLER response handler callback member function of OBJ
#define CF_CREATE_CLIENT_CLASS(IFACE, CLINAME, SVCTYPE, CLASS, RSPHANDLER) \
    CubeFramework::CallbackContainer<SVCTYPE##_cxx_iface::c_rsp_type>::ObjCallback<CLASS> CLINAME##_callback{RSPHANDLER}; \
    CubeFramework::Client<SVCTYPE##_cxx_iface, CubeFramework::Node<0>> CLINAME{IFACE, CLINAME##_callback};

/// @brief create a client instance with indexed Node
/// @param IFACE interface instance name
/// @param ID Node index
/// @param CLINAME client instance name
/// @param SVCTYPE service type
/// @param CLASS class name
/// @param RSPHANDLER response handler callback member function of OBJ
#define CF_CREATE_CLIENT_CLASS_INDEX(IFACE, ID, CLINAME, SVCTYPE, CLASS, RSPHANDLER) \
    CubeFramework::CallbackContainer<SVCTYPE##_cxx_iface::c_rsp_type>::ObjCallback<CLASS> CLINAME##_callback{RSPHANDLER}; \
    CubeFramework::Client<SVCTYPE##_cxx_iface, CubeFramework::Node<ID>> CLINAME{IFACE, CLINAME##_callback};
