#include <Alarm.hpp>
#include <IO/Logger.hpp>
#include <Scheduler/Semaphore.hpp>
#include <Scheduler/Thread.hpp>

#define ITERATIONS 20
#define SLEEP 1000000000

int teste0(void *ptr) {
    Semaphore *semaphore = (Semaphore *)ptr;
    semaphore->p();
    int i = 0;
    while (i < ITERATIONS) {
        int j = SLEEP;
        while (j--);
        Logger::log("Thread0 %d\n", i);
        i++;
    }
    semaphore->v();
    return 0;
}

int teste1(void *ptr) {
    Semaphore *semaphore = (Semaphore *)ptr;
    semaphore->p();
    int i = 0;
    while (i < ITERATIONS) {
        int j = SLEEP;
        while (j--);
        Logger::log("Thread1 %d\n", i);
        i++;
    }
    semaphore->v();
    return 0;
}

int main(void *) {
    Logger::log("APP\n");
    Alarm::delay(1);
    Semaphore semaphore(1);
    Thread *thread0 = new (Memory::APPLICATION) Thread(teste0, &semaphore, Thread::Priority::NORMAL);
    Thread *thread1 = new (Memory::APPLICATION) Thread(teste1, &semaphore, Thread::Priority::NORMAL);
    // Thread::yield();
    Thread::join(thread0);
    Thread::join(thread1);
    delete thread0;
    delete thread1;
    return 0;
}
