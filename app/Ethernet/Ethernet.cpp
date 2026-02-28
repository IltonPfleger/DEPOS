#include <Semaphore.hpp>
#include <Spin.hpp>
#include <Thread.hpp>
#include <machine/Machine.hpp>
#include <network/NIC.hpp>
#include <network/ethernet/Ethernet.hpp>
#include <utils/Lists.hpp>
#include <utils/Observer.hpp>

using namespace DEPOS;

// Semaphore *console;
// volatile bool start = false;
//
typedef Meta::GetFromTypeList<Traits<Ethernet>::Devices, 0>::Result Device;
typedef NIC<Ethernet, Device> Network;
//
// void *receiver(void *id) {
//     auto net = NIC::instance();
//     while (!start)
//         ;
//     while (1) {
//         auto buffer = net->receive();
//         console->p();
//         Console::cout << (long)id << " Received " << buffer->length() << Console::endl;
//         console->v();
//         net->release(buffer);
//     }
// }

int main(int, char *[]) {

    Network::init();

    while (1)
        ;

    // Device::init();

    // NIC<Device>::init();

    // console = new Semaphore(0);

    // for (long i = 0; i < 10; i++)
    //     new Thread(receiver, (void *)i);

    // start = true;
    // console->v();

    // return 0;
}
