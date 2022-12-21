#include <stdint.h>

namespace CubeFramework {

template <typename c_msg_type>
class CallbackContainer {
public:
    /// @brief Base class for broadcast message callbacks.
    class Callback {
    public:
        virtual void operator()(const CanardRxTransfer& transfer, const c_msg_type& msg) = 0;
    };

    /// @brief Static callback class.
    class StaticCallback : public Callback {
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
    template <typename T>
    class ObjCallback : public Callback {
    public:
        constexpr ObjCallback(void (T::*_cb)(const CanardRxTransfer& transfer, const c_msg_type& msg)) : cb(_cb) {}
        void operator()(const CanardRxTransfer& transfer, const c_msg_type& msg) override {
            if (obj != nullptr) {
                (obj->*cb)(transfer, msg);
            }
        }
        // bind
        void bind(T* _obj) {
            obj = _obj;
        }
    private:
        T *obj = nullptr;
        void (T::*cb)(const CanardRxTransfer& transfer, const c_msg_type& msg);
    };
};

} // namespace CubeFramework

#define CF_BIND(HANDLERNAME, OBJ) HANDLERNAME##_callback.bind(OBJ);
