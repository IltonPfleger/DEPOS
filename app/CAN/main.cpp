#include <Alarm.hpp>
#include <machine/Machine.hpp>
#include <utility/Console.hpp>

using namespace DEPOS;

int main(int, char *[]) {
    using Device = IPMSCANFD<CAN0>;

    // static uintmax_t id   = 0;
    // static uintmax_t data = 0;

    auto device = Device::instance();

    while (1) {
        // Console::println("OI");
        // NetworkBuffer *raw  = device.alloc(8);
        // CAN::Buffer *buffer = static_cast<CAN::Buffer *>(raw);
        // new (buffer) CAN::Buffer(0, false, false, 8);
        //// memset(buffer->data(), 0, 8);
        ////*buffer->data<unsigned int *>() = 0;
        ////*(buffer->data<unsigned int *>() + 1) = 0;
        // device.send(buffer);

        //   Buffer b0(id);
        //*b0.data<decltype(&data)>() = data;
        //   b0.length(sizeof(data));
        //   can.send(&b0);

        NetworkBuffer *raw = device.doReceive();

        if (raw) {
            CAN::Buffer *buffer = static_cast<CAN::Buffer *>(raw);
            Console::println("ID: ", Console::Hex(buffer->id()));
            Console::println("Length: ", buffer->length());
            Console::println("Data: ", Console::Hex(*buffer->data<uintptr_t *>()));
            device.doRelease(raw);
        }
    }

    return 0;
}
