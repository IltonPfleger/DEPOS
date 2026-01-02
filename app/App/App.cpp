//// #include <Alarm.hpp>
//// #include <Machine.hpp>
// #include <Semaphore.hpp>
// #include <Thread.hpp>
// #include <memory/Memory.hpp>
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

    // mutex.p();
    // for (unsigned long i = 0; i < N; i++) {
    //     threads[i] = new Thread(thread_function, (void *)(long long)i, Thread::Criterion::NORMAL);
    // }
    // Console::out << "Created!\n";
    // mutex.v();

    // for (unsigned long i = 0; i < N; i++)
    //     Thread::join(*threads[i]);

    // for (int i = 0; i < N; i++)
    //     delete threads[i];

    Console::out << "\nApplication Done!\n";
    return 0;
}
