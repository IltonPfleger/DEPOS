// #include <kernel/Alarm.hpp>
#include <machine/Machine.hpp>
// #include <network/Ethernet.hpp>
// #include <network/NetworkServicesManager.hpp>
#include <network/protocols/TFTP.hpp>
// #include <network/protocols/UDP.hpp>
//  #include <shared/console/Console.hpp>
#include <types.hpp>

using namespace DEPOS;

int main(int, char *[]) {
    typedef Meta::GetFromTypeList<Traits<Ethernet>::Devices, 0>::Result Device;

    ARP *router = new _ARP<Device, IPv4>(Device::instance());
    // router->bind(IPv4::Address(192, 168, 1, 1));
    // router->resolve(IPv4::Address(192, 168, 1, 100));

    IPv4 *ipv4 = new IPv4(Device::instance(), router);

    UDP *udp = new UDP(ipv4, 0);

    TFTP *tftp = new TFTP(*udp, IPv4::Address(192, 168, 1, 100));

    Alarm::udelay(100);

    auto size    = 1 << 10;
    auto *buffer = new unsigned char[size];
    tftp->request("Image", buffer, size);

    while (1)
        ;

    // udp->send(IPv4::Address(192, 168, 1, 100), 0, udp->alloc(100));

    // while (1) {
    //     NetworkBuffer buffer = udp->receive();
    //     for (size_t i = 0; i < buffer.length(); i++) {
    //         Console::cout << static_cast<char>(buffer[i]);
    //     }
    //     Console::cout << Console::endl;
    // }

    // router->resolve(IPv4::Address{192, 168, 1, 100});

    // Console::cout << "SOLVED!" << Console::endl;

    //// Ethernet
    // Device device;

    //// typedef Device::Address HA;
    //// NetworkBuffer *b1 = device.alloc(500);
    //// device.send(HA(0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF), HA(0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF), *b1);
    //// device.free(b1);

    // typedef DEPOS::ARP<Ethernet, IPv4> ARP;
    // arp.bind(ARP::PA(Traits<IPv4>::Address));

    return 0;
}
