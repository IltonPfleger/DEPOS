#include <PeriodicThread.hpp>
#include <architecture/Timer.hpp>

using namespace DEPOS;

Thread::Return init(Thread::Argument) {
	PeriodicThread::wait();
    while (1) {
        volatile unsigned int i = 500;
        while (i)
            i = i - 1;
        PeriodicThread::wait();
    }
    return Thread::Return{};
}

int main() { new PeriodicThread(init, 0, Thread::Criterion::NORMAL, 1000); }
