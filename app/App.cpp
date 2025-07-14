#include <Alarm.hpp>
#include <IO/Logger.hpp>
#include <Memory.hpp>
#include <Semaphore.hpp>
#include <Thread.hpp>

static constexpr int N          = 5;
static constexpr int ITERATIONS = 2;

Thread *threads[N];

int thread_function(void *arg) {
    int id = (int)(long long)arg;
    int i  = ITERATIONS;
    while (i--) {
        Logger::println("THREAD %d\n", id);
        Alarm::usleep(1000);
    }
    return 0;
}

int main(void *) {
    Logger::println("Application: \n");

    for (int i = 0; i < N; i++) {
        threads[i] = new (Memory::APPLICATION) Thread(thread_function, (void *)(long long)i, Thread::Priority::NORMAL);
    }

    for (int i = 0; i < N; i++) {
        Thread::join(threads[i]);
        delete threads[i];
    }

    Logger::println("Application Done!\n");
    return 0;
}
