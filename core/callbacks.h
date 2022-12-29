#include <stdint.h>

namespace CubeFramework {

/// @brief Base class for broadcast message callbacks.
template <typename c_msg_type>
class Callback {
public:
    virtual void operator()(const CanardRxTransfer& transfer, const c_msg_type& msg) = 0;
};

/// @brief Static callback class.
template <typename c_msg_type>
class StaticCallback : public Callback<c_msg_type> {
public:
    StaticCallback(void (*_cb)(const CanardRxTransfer& transfer, const c_msg_type& msg)) : cb(_cb) {}
    // delete default constructor
    StaticCallback() = delete;

    void operator()(const CanardRxTransfer& transfer, const c_msg_type& msg) override {
        cb(transfer, msg);
    }
private:
    void (*cb)(const CanardRxTransfer& transfer, const c_msg_type& msg);
};

/// @brief Object callback class.
/// @tparam T type of object to call the callback on
template <typename T, typename c_msg_type>
class ObjCallback : public Callback<c_msg_type> {
public:
    constexpr ObjCallback(T* _obj, void (T::*_cb)(const CanardRxTransfer& transfer, const c_msg_type& msg)) : obj(_obj), cb(_cb) {}
    void operator()(const CanardRxTransfer& transfer, const c_msg_type& msg) override {
        if (obj != nullptr) {
            (obj->*cb)(transfer, msg);
        }
    }
private:
    T *obj = nullptr;
    void (T::*cb)(const CanardRxTransfer& transfer, const c_msg_type& msg);
};

} // namespace CubeFramework
