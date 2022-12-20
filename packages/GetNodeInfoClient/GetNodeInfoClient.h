#include <core/types.h>

class GetNodeInfoClient : public AbstractPackage {
    GetNodeInfoClient();
    REGISTER_SERVICE_CLIENT(GetNodeInfoClient, uavcan_protocol_GetNodeInfo);

    // singleton
    static GetNodeInfoClient instance;
public:
    static GetNodeInfoClient& instance() {
        return instance;
    }

    static void handle_uavcan_protocol_GetNodeInfoResponse(const uavcan_protocol_GetNodeInfoResponse& msg);
}

namespace CubeFramework {
    GetNodeInfoClient& get_node_info_client();
}
