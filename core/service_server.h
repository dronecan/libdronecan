#pragma once
#include "handler_list.h"
#include "interface.h"

namespace CubeFramework {

template <typename svctype, typename handlerlist>
class Server : public Handler {

public:
    Server(Interface &_interface, Callback<typename svctype::c_req_type> &_cb) : 
    interface(_interface),
    cb(_cb) {
        // register the listener for the service request
        static handlerlist _handler_list(CanardTransferTypeRequest, svctype::ID, svctype::SIGNATURE);
        handler_list = &_handler_list;
        handler_list->set_branch_head(this);
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
            return interface.respond(transfer.source_node_id, rsp_transfer);
        }
        return false;
    }

private:
    Transfer rsp_transfer;
    uint8_t rsp_buf[svctype::RSP_MAX_SIZE];
    Interface &interface;
    Callback<typename svctype::c_req_type> &cb;

    handlerlist *handler_list;

    uint8_t priority = CANARD_TRANSFER_PRIORITY_MEDIUM;
    uint8_t transfer_id = 0;
#if CANARD_MULTI_IFACE
    uint8_t iface_mask = CANARD_IFACE_ALL;
#endif
};

} // namespace CubeFramework

/// Helper macros to create server instances

/// @brief create a server instance with indexed handler_list
/// @param IFACE interface instance name
/// @param ID handler_list instance id
/// @param SRVNAME server instance name
/// @param SVCTYPE service type name
/// @param REQHANDLER request handler function
#define CF_CREATE_SERVER_INDEX(IFACE, ID, SRVNAME, SVCTYPE, REQHANDLER) \
    CubeFramework::StaticCallback<SVCTYPE##_cxx_iface::c_req_type> SRVNAME##_callback{REQHANDLER}; \
    CubeFramework::Server<SVCTYPE##_cxx_iface, CubeFramework::HandlerList<ID>> SRVNAME{IFACE, SRVNAME##_callback};

/// @brief create a server instance
/// @param IFACE interface instance name
/// @param SRVNAME server instance name
/// @param SVCTYPE service type name
/// @param REQHANDLER request handler function
#define CF_CREATE_SERVER(IFACE, SRVNAME, SVCTYPE, REQHANDLER) \
    CubeFramework::StaticCallback<SVCTYPE##_cxx_iface::c_req_type> SRVNAME##_callback{REQHANDLER}; \
    CubeFramework::Server<SVCTYPE##_cxx_iface, CubeFramework::HandlerList<0>> SRVNAME{IFACE, SRVNAME##_callback};

/// @brief create a client instance
/// @param IFACE interface instance name
/// @param SRVNAME server instance name
/// @param SVCTYPE service type
/// @param CLASS class name
/// @param REQHANDLER request handler callback member function of OBJ
#define CF_CREATE_SERVER_CLASS(IFACE, SRVNAME, SVCTYPE, CLASS, REQHANDLER) \
    CubeFramework::ObjCallback<CLASS, SVCTYPE##_cxx_iface::c_req_type> SRVNAME##_callback{this, REQHANDLER}; \
    CubeFramework::Server<SVCTYPE##_cxx_iface, CubeFramework::HandlerList<0>> SRVNAME{IFACE, SRVNAME##_callback};

/// @brief create a client instance with indexed HandlerList
/// @param IFACE interface instance name
/// @param ID HandlerList index
/// @param SRVNAME server instance name
/// @param SVCTYPE service type
/// @param CLASS class name
/// @param REQHANDLER request handler callback member function of OBJ
#define CF_CREATE_SERVER_CLASS_INDEX(IFACE, ID, SRVNAME, SVCTYPE, CLASS, REQHANDLER) \
    CubeFramework::ObjCallback<CLASS, SVCTYPE##_cxx_iface::c_req_type> SRVNAME##_callback{this, REQHANDLER}; \
    CubeFramework::Server<SVCTYPE##_cxx_iface, CubeFramework::HandlerList<ID>> SRVNAME{IFACE, SRVNAME##_callback};
