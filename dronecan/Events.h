#include <stdint.h>

// Abstract Events class
namespace DroneCAN
{
class EventObject
{
public:
    // @return true if event was signaled, false if failed to signal
    virtual bool signal() = 0;
};

class Events
{
public:
    // @param event: event to register
    // @return true if event was registered, false if failed to register
    virtual bool register_event(EventObject &) = 0;

    // @param event: event to unregister
    virtual void unregister_event(EventObject &) = 0;

    // @param events: events to check
    // @param timeout: timeout in milliseconds
    // @return true if event was signaled, false if timeout
    virtual bool wait_any(EventObject *events[], uint32_t num_events, uint32_t timeout) = 0;

    // @param events: events to check
    // @param timeout: timeout in milliseconds
    // @return true if event was signaled, false if timeout
    virtual bool wait_all(EventObject *events[], uint32_t num_events, uint32_t timeout) = 0;

    // @param events: events to check
    // @param timeout: timeout in milliseconds
    // @return true if event was signaled, false if timeout
    virtual bool wait_one(EventObject *events[], uint32_t num_events, uint32_t timeout) = 0;
};

} // namespace DroneCAN
