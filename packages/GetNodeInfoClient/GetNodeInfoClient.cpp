#include <GetNodeInfoClient.h>

static GetNodeInfoClient::GetNodeInfoClient instance;

GetNodeInfoClient get_node_info_client() {
    return GetNodeInfoClient::instance();
}
