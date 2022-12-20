#include <core/message_registry.h>
#include <iostream>

#define TEST_MESSAGE_ID 1
#define TEST_MESSAGE_SIGNATURE 0x1234567890ABCDEFULL

struct test_message {
    uint8_t a;
    uint8_t b;
};

bool test_message_decode(const CanardRxTransfer* transfer, test_message* msg) {
    std::cout << "test_message_decode: " << std::endl;
    return true;
}

class subscriber {
public:
    static void handle_test_message(const CanardRxTransfer& transfer, const test_message& msg) {
        std::cout << "subscriber::handle_test_message: " << std::endl;
    }
};


REGISTER_MESSAGE_DECLARE(subscriber, test_message);

int main(int argc, char *argv[]) {
    std::cout << "main: " << std::endl;

    test_message msg;
    msg.a = 1;
    msg.b = 2;

    CanardRxTransfer transfer;
    transfer.data_type_id = TEST_MESSAGE_ID;
    transfer.transfer_id = 0;

    uint64_t signature = 0;
    if (TrunkRegistry::accept_message(TEST_MESSAGE_ID, signature)) {
        std::cout << "main: accept_message: " << std::endl;
        TrunkRegistry::handle_message((uint8_t*)&msg, 2, transfer);
    }

    return 0;
}
