#include <Alarm.hpp>
#include <network/ethernet/ip/UDP.hpp>
#include <utils/Console.hpp>

class UDP {
    typedef Meta::GetFromTypeList<Traits<Ethernet>::Devices, 0>::Result NIC;

  public:
    static void init() {
        TraceIn();

        NIC::init();

        Alarm::delay(1);

        IPv4 ipv4;

        unsigned int max = 64 * 1024;
        unsigned char *buffer = new unsigned char[max];

        while (1) {
            unsigned int size = ipv4.receive(buffer, max);
            Console::print(size, '\n');

            for (size_t i = 0; i < size; i += 4) {
                Console::print(Console::Hex(*((unsigned short *)(buffer + i))));
                Console::print(" ");

                if (i != 0 && i % 32 == 0) {
                    Console::print("\n");
                }
            }
        }
        delete[] buffer;
        TraceOut();
    }
};

int main(int, char *[]) {
    UDP::init();
    return 0;
}
