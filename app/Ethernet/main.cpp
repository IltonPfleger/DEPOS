// #include <kernel/Alarm.hpp>
#include <machine/Machine.hpp>
// #include <network/Ethernet.hpp>
// #include <network/NetworkServicesManager.hpp>
#include <network/protocols/ARP.hpp>
// #include <network/protocols/IPv4.hpp>
//// #include <network/protocols/TFTP.hpp>
//// #include <network/protocols/UDP.hpp>
// #include <shared/console/Console.hpp>
#include <types.hpp>

using namespace DEPOS;

int main(int, char *[]) {
    typedef Meta::GetFromTypeList<Traits<Ethernet>::Devices, 0>::Result Device;

    auto *router = new ARP(Device::instance(), {192, 168, 1, 1});
    router->resolve({192, 168, 1, 100});

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
