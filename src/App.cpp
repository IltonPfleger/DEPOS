#include <Alarm.hpp>
#include <IO/Logger.hpp>
#include <Semaphore.hpp>
#include <Thread.hpp>

#define ITERATIONS 100
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
    Logger::log("APP:\n");
    Logger::log("Delay...");
    Alarm::delay(1);
    Logger::log("Done!\n");
    Semaphore semaphore(1);
    Thread *thread0 = new (Memory::APPLICATION) Thread(teste0, &semaphore, Thread::Priority::NORMAL);
    Thread *thread1 = new (Memory::APPLICATION) Thread(teste1, &semaphore, Thread::Priority::NORMAL);
    Thread::join(thread0);
    Thread::join(thread1);
    delete thread0;
    delete thread1;
    return 0;
}
