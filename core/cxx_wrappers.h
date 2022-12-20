
/// @brief Broadcast message interface.
/// @param MSGTYPE message type name
/// @param MSG_ID message id
/// @param MSG_SIGNATURE message signature
/// @param MSG_MAX_SIZE maximum message size
#define BROADCAST_MESSAGE_CXX_IFACE(MSGTYPE, MSG_ID, MSG_SIGNATURE, MSG_MAX_SIZE) \
    class MSGTYPE##_cxx_iface { \
    public: \
        typedef MSGTYPE c_msg_type; \
        static constexpr uint32_t (*encode)(c_msg_type*, uint8_t*) = MSGTYPE##_encode; \
        static constexpr bool (*decode)(const CanardRxTransfer* transfer, c_msg_type*) = MSGTYPE##_decode; \
        static constexpr uint16_t ID = MSG_ID; \
        static constexpr uint64_t SIGNATURE = MSG_SIGNATURE; \
        static constexpr uint16_t MAX_SIZE = MSG_MAX_SIZE; \
    };

/// @brief Service message interface.
/// @param SVCTYPE message type name
/// @param SVC_ID message id
/// @param SVC_SIGNATURE message signature
/// @param SVC_REQUEST_MAX_SIZE maximum request size
/// @param SVC_RESPONSE_MAX_SIZE maximum response size
#define SERVICE_MESSAGE_CXX_IFACE(SVCTYPE, SVC_ID, SVC_SIGNATURE, SVC_REQUEST_MAX_SIZE, SVC_RESPONSE_MAX_SIZE) \
    class SVCTYPE##_cxx_iface { \
    public: \
        typedef SVCTYPE##Request c_req_type; \
        typedef SVCTYPE##Response c_rsp_type; \
        static constexpr uint32_t (*req_encode)(c_req_type*, uint8_t*) = SVCTYPE##Request_encode; \
        static constexpr uint32_t (*rsp_encode)(c_rsp_type*, uint8_t*) = SVCTYPE##Response_encode; \
        static constexpr bool (*req_decode)(const CanardRxTransfer* transfer, c_req_type*) = SVCTYPE##Request_decode; \
        static constexpr bool (*rsp_decode)(const CanardRxTransfer* transfer, c_rsp_type*) = SVCTYPE##Response_decode; \
        static constexpr uint16_t ID = SVC_ID; \
        static constexpr uint64_t SIGNATURE = SVC_SIGNATURE; \
        static constexpr uint16_t REQ_MAX_SIZE = SVC_REQUEST_MAX_SIZE; \
        static constexpr uint16_t RSP_MAX_SIZE = SVC_RESPONSE_MAX_SIZE; \
    };
