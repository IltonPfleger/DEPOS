#include <Alarm.hpp>
// #include <network/ethernet/Ethernet.hpp>
#include <machine/Machine.hpp>
#include <utils/Console.hpp>
#include <utils/Observer.hpp>

typedef Meta::GetFromTypeList<Traits<Ethernet>::Devices, 0>::Result NIC;

// static void init() {
//     TraceIn();
//
//     NIC::init();
//
//     Alarm::delay(1);
//
//     UDP protocol;
//
//     unsigned int max = 64 * 1024;
//     unsigned char *buffer = new unsigned char[max];
//
//     while (1) {
//         unsigned int size = protocol.receive(buffer, max);
//         Console::print(size, '\n');
//
//         for (size_t i = 0; i < size; i += 4) {
//             Console::print(Console::Hex(*((unsigned short *)(buffer + i))));
//             Console::print(" ");
//
//             if (i != 0 && i % 32 == 0) {
//                 Console::print("\n");
//             }
//         }
//     }
//     delete[] buffer;
//     TraceOut();
// }

class ARP : public Observer {

    void update() { TraceIn(); }

  public:
    ARP() = default;
};

int main(int, char *[]) {
    TraceIn();

    NIC::init();

    auto nic = NIC::instance();

    ARP arp;

    nic->attach(&arp);

    //nic->debug();

    Alarm::delay(10000);

    TraceOut();

    return 0;
}
