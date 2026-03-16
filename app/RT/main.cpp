#include <PeriodicThread.hpp>
#include <architecture/Timer.hpp>

using namespace DEPOS;

Thread::Return init(Thread::Argument) {
    // Microsecond last;
    while (1) {
        volatile unsigned int i = 500;
        while (i)
            i = i - 1;
        // last = Timer::now();
        PeriodicThread::wait();
        // Microsecond now = Timer::now();
        // Console::cout << now - last << Console::endl;
        // last = now;
    }
    return Thread::Return{};
}

int main() { new PeriodicThread(init, 0, Thread::Criterion::NORMAL, 1000); }
