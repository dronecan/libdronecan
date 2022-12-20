#pragma once
#include "node.h"
#include "interface.h"

namespace CubeFramework {

template <typename svctype, typename svcnode>
class Server : public Handler {

public:
    Server(typename CallbackContainer<typename svctype::c_req_type>::Callback &_cb) : 
    cb(_cb) {
        // register the listener for the service request
        static svcnode _node(CanardTransferTypeRequest, svctype::ID, svctype::SIGNATURE);
        node = &_node;
        node->set_branch_head(this);
    }

    // delete copy constructor and assignment operator
    Server(const Server&) = delete;

    void handle_message(const CanardRxTransfer& transfer) {
        typename svctype::c_req_type msg {};
        svctype::req_decode(&transfer, &msg);
        transfer_id = transfer.transfer_id;
        // call the registered callback
        cb(transfer, msg);
    }

    /// @brief Send a response to the request
    /// @param transfer transfer object of the request
    /// @param msg message containing the response
    /// @return true if the response was put into the queue successfully
    bool respond(const CanardRxTransfer& transfer, typename svctype::c_rsp_type& msg) {
        // encode the message
        uint16_t len = svctype::rsp_encode(&msg, rsp_buf);
        // send the message if encoded successfully
        if (len > 0) {
#if CANARD_ENABLE_CANFD
            rsp_transfer.canfd = transfer.canfd;
#endif
#if CANARD_MULTI_IFACE
            rsp_transfer.iface_mask = iface_mask;
#endif
            rsp_transfer.transfer_type = CanardTransferTypeResponse;
            rsp_transfer.inout_transfer_id = &transfer_id;
            rsp_transfer.data_type_id = svctype::ID;
            rsp_transfer.data_type_signature = svctype::SIGNATURE;
            rsp_transfer.payload = rsp_buf;
            rsp_transfer.payload_len = len;
            rsp_transfer.priority = transfer.priority;
            return node->interface().respond(transfer.source_node_id, rsp_transfer);
        }
        return false;
    }

private:
    Transfer rsp_transfer;
    uint8_t rsp_buf[svctype::RSP_MAX_SIZE];
    typename CallbackContainer<typename svctype::c_req_type>::Callback &cb;

    svcnode *node;

    uint8_t priority = CANARD_TRANSFER_PRIORITY_MEDIUM;
    uint8_t transfer_id = 0;
#if CANARD_MULTI_IFACE
    uint8_t iface_mask = CANARD_IFACE_ALL;
#endif
};

} // namespace CubeFramework

/// Helper macros to create server instances

/// @brief create a server instance with indexed node
/// @param ID node instance id
/// @param SRVNAME server instance name
/// @param SVCTYPE service type name
/// @param REQHANDLER request handler function
#define CF_CREATE_SERVER_INDEX(ID, SRVNAME, SVCTYPE, REQHANDLER) \
    CubeFramework::CallbackContainer<SVCTYPE##_cxx_iface::c_req_type>::StaticCallback SRVNAME##_callback{REQHANDLER}; \
    CubeFramework::Server<SVCTYPE##_cxx_iface, CubeFramework::Node<ID>> SRVNAME{SRVNAME##_callback};

/// @brief create a server instance
/// @param SRVNAME server instance name
/// @param SVCTYPE service type name
/// @param REQHANDLER request handler function
#define CF_CREATE_SERVER(SRVNAME, SVCTYPE, REQHANDLER) \
    CubeFramework::CallbackContainer<SVCTYPE##_cxx_iface::c_req_type>::StaticCallback SRVNAME##_callback{REQHANDLER}; \
    CubeFramework::Server<SVCTYPE##_cxx_iface, CubeFramework::Node<0>> SRVNAME{SRVNAME##_callback};

/// @brief create a server instance with indexed node and object instance
/// @param ID node instance id
/// @param SRVNAME server instance name
/// @param SVCTYPE service type name
/// @param OBJ object instance
/// @param REQHANDLER request handler member function of object instance
#define CF_CREATE_SERVER_INDEX_OBJ(ID, SRVNAME, SVCTYPE, OBJ, REQHANDLER) \
    CubeFramework::Server<SVCTYPE, CubeFramework::Node<ID>> \
    SRVNAME(CubeFramework::Server::ObjCallback(OBJ, REQHANDLER));

/// @brief create a server instance with object instance
/// @param SRVNAME server instance name
/// @param SVCTYPE service type name
/// @param OBJ object instance
/// @param REQHANDLER request handler member function of object instance
#define CF_CREATE_SERVER_OBJ(SRVNAME, SVCTYPE, OBJ, REQHANDLER) \
    CubeFramework::Server<SVCTYPE, CubeFramework::Node<0>> \
    SRVNAME(CubeFramework::Server::ObjCallback(OBJ, REQHANDLER));
