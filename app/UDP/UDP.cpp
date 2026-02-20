#include <Alarm.hpp>
#include <machine/Machine.hpp>
// #include <network/ethernet/NIC.hpp>
// #include <network/ethernet/ip/ARP.hpp>
// #include <network/ethernet/ip/IPv4.hpp>
// #include <network/ethernet/ip/TFTP.hpp>
// #include <network/ethernet/ip/UDP.hpp>
#include <utils/Console.hpp>
// #include <utils/Observer.hpp>

int main(int, char *[]) {
    TraceIn();

    typedef Meta::GetFromTypeList<Traits<Ethernet>::Devices, 0>::Result Driver;

    Driver::init();

    // auto buffer = Driver::instance()->alloc();

    // constexpr unsigned int size = 1 << 20;
    // void *buffer = Memory::alloc(size);
    unsigned int i = 1000;
    (void)i;
    while (1) {
        // Driver::instance()->send(buffer, 1400);
    }

    // void *buffer = Memory::alloc(size);
    // TFTP<Driver> tftp("192.168.1.100");
    // tftp.request("Image", buffer, size);

    // while (1)
    //     ;

    // ARP<Driver> *arp = ARP<Driver>::instance();
    //// NIC<Driver> *nic = NIC<Driver>::instance();

    // UDP<Driver> udp;
    // volatile unsigned int i = 10'000'000;
    // while (i) {
    //     i = i - 1;
    // }
    // unsigned char buffer[1518];
    // udp.send("255.255.255.255", 0, buffer, 64);

    // ARP<Driver>::release();

    // NIC<Driver>::release();

    // NIC<Driver> nic;
    //  ARP<Driver> arp(&nic);
    //  IPv4<Driver> ipv4(&nic, &arp);

    //  arp.resolve("192.168.1.100");
    //(void)nic;
    //(void)arp;

    TraceOut();

    return 0;
}
