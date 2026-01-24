//// #include <Alarm.hpp>
#include <machine/Machine.hpp>
// #include <Semaphore.hpp>
// #include <Thread.hpp>
// #include <memory/Memory.hpp>
#include <Alarm.hpp>
#include <network/ethernet/ip/UDP.hpp>
#include <utils/Console.hpp>
//
// static constexpr unsigned long N = 10;
// static Thread *threads[N];
// static constexpr int ITERATIONS = 5;
//
// static Semaphore mutex;
//
// int thread_function(void *arg) {
//     int id = (int)(long long)arg;
//     int i = ITERATIONS;
//     while (i--) {
//         mutex.p();
//         Console::out << id << " ";
//         mutex.v();
//         Thread::yield();
//         //     Alarm::usleep(10000);
//     }
//     return 0;
// }
//

// static unsigned char *create_frame(unsigned char *frame, unsigned int length) {
//     unsigned int offset = 0;
//
//     for (int i = 0; i < 6; i++)
//         frame[offset++] = 0xFF;
//
//     unsigned char src[6] = {0x00, 0x11, 0x22, 0x33, 0x44, 0x55};
//     for (int i = 0; i < 6; i++)
//         frame[offset++] = src[i];
//
//     frame[offset++] = 0x08;
//     frame[offset++] = 0x00;
//
//     const char payload[] = "Hello World!";
//     for (unsigned int i = 0; i < sizeof(payload) - 1; i++)
//         frame[offset++] = payload[i];
//
//     for (; offset < length; offset++)
//         frame[offset] = 0;
//     return frame;
// }

int main(int, char *[]) {
    //using NIC = Meta::GetFromTypeList<Traits<Ethernet>::Devices, 0>::Result;
    //NIC::init();
    Alarm::delay(1);

    //constexpr unsigned int N = 300;
    //unsigned char data[N];
    //Ethernet::Address destination({0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF});
    //Ethernet::Address source({0x00, 0x00, 0x00, 0x00, 0x00, 0x00});
    //Ethernet::Frame frame(destination, source, 0, data, N);

    //auto nic = NIC::instance();

    //nic->send(frame, N);

    // unsigned int length = 100;
    // unsigned char *frame = new (Heap::SYSTEM) unsigned char[length];
    // while (1) {
    //     nic->send(frame, length);

    //    unsigned int received = nic->receive(frame, length);

    //    if (received == 0)
    //        continue;

    //    for (unsigned int i = 0; i < received / 2; i++) {
    //        if ((i + 1) % 16 == 0)
    //            Console::print('\n');
    //        Console::println("0x%x ", __builtin_bswap16(frame[i]));
    //    }

    //    Console::print('\n');
    //}
    return 0;
}
