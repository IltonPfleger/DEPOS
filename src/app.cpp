#include <IO/Logger.hpp>
#include <Semaphore.hpp>
#include <Thread.hpp>

#define ITERATIONS 10
#define SLEEP 1000000

// static Semaphore semaphore;

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
    // Alarm::delay(1);
    Semaphore semaphore(1);
    Thread::Thread *thread0 = new (Memory::APPLICATION) Thread::Thread(teste0, &semaphore, Thread::Priority::NORMAL);
    Thread::Thread *thread1 = new (Memory::APPLICATION) Thread::Thread(teste1, &semaphore, Thread::Priority::NORMAL);
    // Semaphore::create(&semaphore, 1);
    Thread::join(thread0);
    Thread::join(thread1);
    delete thread0;
    delete thread0;
    return 0;
}
