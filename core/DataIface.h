#pragma once

#include "CanardIface.h"

namespace CubeFramework {

// DataIface to CanardIface
class DataIface {
public:
    DataIface(CanardIface &canard_iface) : canard_iface(canard_iface) {}

    virtual void init() = 0;
    virtual bool send(const CanardCANFrame &frame) = 0;
    virtual void update_rx() = 0;

protected:
    CanardIface &canard_iface;
};

} // namespace CubeFramework
