#include <Alarm.hpp>
#include <machine/Machine.hpp>
#include <utility/Console.hpp>

using namespace QUARK;

int main(int, char *[]) {
    using Device = IPMSCANFD<CAN0>;

    auto device = Device::instance();

    while (1) {
        unsigned char data[] = {0, 1, 2, 4};
        device.send(0, data, 4);

        NetworkBuffer *raw = device.doReceive();

        if (raw) {
            CAN::Buffer *buffer = static_cast<CAN::Buffer *>(raw);
            Console::println("ID: ", Console::Hex(buffer->id()));
            Console::println("Length: ", buffer->length());
            Console::println("Data: ", Console::Hex(*buffer->data<uintptr_t *>()));
            device.release(raw);
        }
    }
    return 0;
}
