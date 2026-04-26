#include <drivers/network/can/StarFiveCAN.hpp>
#include <kernel/Alarm.hpp>
#include <shared/console/Console.hpp>

using namespace DEPOS;

int main(int, char *[]) {
    using Device = StarFiveCAN<CAN0>;
    using Buffer = CAN::Buffer;

    static uintmax_t id   = 0;
    static uintmax_t data = 0;

    Device can;

    while (1) {
        Buffer b0(id);
        *b0.data<decltype(&data)>() = data;
        b0.length(sizeof(data));
        can.send(&b0);

        const NetworkBuffer *b1 = can.receive();

        if (b1) {
            const Buffer *b2 = static_cast<const Buffer *>(b1);
            Console::println("ID: ", b2->id());
            Console::println("Length: ", b2->length());
            Console::println("Data: ", *b2->data<decltype(&data)>());
        }

        data++;
        id++;
        Alarm::udelay(1'000'000);
    }

    return 0;
}
