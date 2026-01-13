//// #include <Alarm.hpp>
//// #include <Machine.hpp>
// #include <Semaphore.hpp>
// #include <Thread.hpp>
// #include <memory/Memory.hpp>
#include <Alarm.hpp>
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
int main(int, char *[]) {
    Console::out << "Application: \n";
    Console::out << "Waiting...: \n";
    Alarm::delay(1);
    Console::out << "1,";
    Alarm::delay(1);
    Console::out << "2,";
    Alarm::delay(1);
    Console::out << "3.\n";
    Alarm::delay(1);
    Console::out << "Application Done!\n";
    return 0;
}
