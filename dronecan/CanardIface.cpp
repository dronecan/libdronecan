#include "CanardIface.h"

using namespace DroneCAN;
using namespace Canard;

void CanardIface::init(void* mem_arena, size_t mem_arena_size)
{
    canardInit(&canard, mem_arena, mem_arena_size, onTransferReception, shouldAcceptTransfer, this);
}

bool CanardIface::broadcast(const Transfer &bcast_transfer)
{
    // do canard broadcast
    return canardBroadcast(&canard,
                           bcast_transfer.data_type_signature,
                           bcast_transfer.data_type_id,
                           bcast_transfer.inout_transfer_id,
                           bcast_transfer.priority,
                           bcast_transfer.payload,
                           bcast_transfer.payload_len) > 0;
}

bool CanardIface::request(uint8_t destination_node_id, const Transfer &req_transfer)
{
    // do canard request
    return canardRequestOrRespond(&canard,
                                  destination_node_id,
                                  req_transfer.data_type_signature,
                                  req_transfer.data_type_id,
                                  req_transfer.inout_transfer_id,
                                  req_transfer.priority,
                                  CanardRequest,
                                  req_transfer.payload,
                                  req_transfer.payload_len) > 0;
}

bool CanardIface::respond(uint8_t destination_node_id, const Transfer &res_transfer)
{
    // do canard respond
    return canardRequestOrRespond(&canard,
                                  destination_node_id,
                                  res_transfer.data_type_signature,
                                  res_transfer.data_type_id,
                                  res_transfer.inout_transfer_id,
                                  res_transfer.priority,
                                  CanardResponse,
                                  res_transfer.payload,
                                  res_transfer.payload_len) > 0;
}

void CanardIface::handle_frame(const CanardCANFrame &frame, uint64_t timestamp_usec)
{
    canardHandleRxFrame(&canard, &frame, timestamp_usec);
}

void CanardIface::onTransferReception(CanardInstance* ins, CanardRxTransfer* transfer)
{
    CanardIface* iface = (CanardIface*) ins->user_reference;
    iface->handle_message(*transfer);
}

bool CanardIface::shouldAcceptTransfer(const CanardInstance* ins,
                                       uint64_t* out_data_type_signature,
                                       uint16_t data_type_id,
                                       CanardTransferType transfer_type,
                                       uint8_t source_node_id)
{
    CanardIface* iface = (CanardIface*) ins->user_reference;
    (void) transfer_type;
    (void) source_node_id;
    return iface->accept_message(data_type_id, *out_data_type_signature);
}
