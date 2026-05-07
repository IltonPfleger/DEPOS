// #include <kernel/Alarm.hpp>
#include <machine/Machine.hpp>
// #include <network/Ethernet.hpp>
// #include <network/NetworkServicesManager.hpp>
//// #include <network/protocols/TFTP.hpp>
#include <network/protocols/UDP.hpp>
// #include <shared/console/Console.hpp>
#include <types.hpp>

using namespace DEPOS;

int main(int, char *[]) {
    typedef Meta::GetFromTypeList<Traits<Ethernet>::Devices, 0>::Result Device;

    ARP *router = new _ARP<Device, IPv4>(Device::instance());
    // router->bind(IPv4::Address(192, 168, 1, 1));
    // router->resolve(IPv4::Address(192, 168, 1, 100));

    IPv4 *ipv4 = new IPv4(Device::instance(), router);

    UDP *udp = new UDP(ipv4, 5000);

    int sent = udp->send(IPv4::Address(192, 168, 1, 100), 0, udp->alloc(100));

    Console::cout << "SENT! " << sent << Console::endl;

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
