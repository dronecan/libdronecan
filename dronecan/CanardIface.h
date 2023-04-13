#pragma once

#include <canard/interface.h>
#include <canard.h>

namespace DroneCAN
{

class CanardIface : public Canard::Interface
{

public:
    CanardIface(uint8_t index) :
        Canard::Interface(index) {}

    /// @brief delete copy constructor and assignment operator
    CanardIface(const CanardIface&) = delete;
    CanardIface& operator=(const CanardIface&) = delete;
    CanardIface() = delete;

    void init(void* mem_arena, size_t mem_arena_size);

    /// @brief broadcast message to all listeners on Interface
    /// @param bc_transfer
    /// @return true if message was added to the queue
    bool broadcast(const Canard::Transfer &bcast_transfer) override;

    /// @brief request message from
    /// @param destination_node_id
    /// @param req_transfer
    /// @return true if request was added to the queue
    bool request(uint8_t destination_node_id, const Canard::Transfer &req_transfer) override;

    /// @brief respond to a request
    /// @param destination_node_id
    /// @param res_transfer
    /// @return true if response was added to the queue
    bool respond(uint8_t destination_node_id, const Canard::Transfer &res_transfer) override;

    void handle_frame(const CanardCANFrame &frame, uint64_t timestamp_usec);

    virtual void update_tx(uint64_t timestamp_usec) = 0;

    static void onTransferReception(CanardInstance* ins, CanardRxTransfer* transfer);
    static bool shouldAcceptTransfer(const CanardInstance* ins,
                                     uint64_t* out_data_type_signature,
                                     uint16_t data_type_id,
                                     CanardTransferType transfer_type,
                                     uint8_t source_node_id);

    uint8_t get_node_id() const override
    {
        return canard.node_id;
    }

protected:
    CanardInstance canard {};
};

} // namespace DroneCAN
