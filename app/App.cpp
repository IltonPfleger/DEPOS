#include <Alarm.hpp>
#include <IO/Logger.hpp>
#include <Memory.hpp>
#include <Semaphore.hpp>
#include <Thread.hpp>

static constexpr int N          = 5;
static constexpr int ITERATIONS = 10;

static Thread *threads[N];
static Semaphore *mutex;

int thread_function(void *arg) {
    int id = (int)(long long)arg;
    int i  = ITERATIONS;
    while (i--) {
        mutex->p();
        Logger::println("THREAD: %d | ITERATION: %d\n", id, i);
        Alarm::usleep(1000);
        mutex->v();
    }
    return 0;
}

int main(void *) {
    Logger::println("Application: \n");

    mutex = new (Memory::APPLICATION) Semaphore(1);

    for (int i = 0; i < N; i++) {
        threads[i] = new (Memory::APPLICATION) Thread(thread_function, (void *)(long long)i, Thread::NORMAL);
    }

    for (int i = 0; i < N; i++) Thread::join(threads[i]);

    for (int i = 0; i < N; i++) delete threads[i];

    Logger::println("Application Done!\n");
    return 0;
}
