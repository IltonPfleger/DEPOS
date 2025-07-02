#include <IO/Logger.hpp>
#include <Semaphore.hpp>
#include <Thread.hpp>

#define ITERATIONS 10
#define SLEEP 1000000

static Semaphore semaphore;

int teste0(void *) {
    int i = 0;
    // Semaphore::p(&semaphore);
    while (i < ITERATIONS) {
        int j = SLEEP;
        while (j--);
        Logger::log("Thread0 %d\n", i);
        i++;
    }
    // Semaphore::v(&semaphore);
    return 0;
}

int teste1(void *) {
    int i = 0;
    // Semaphore::p(&semaphore);
    while (i < ITERATIONS) {
        Logger::log("Thread1 %d\n", i);
        int j = SLEEP;
        while (j--);
        i++;
    }
    // Semaphore::v(&semaphore);
    return 0;
}

int main(void *) {
    Logger::log("APP:\n");

    // Thread::Thread thread0;
    // Thread::Thread thread1;

    // Thread::create(&thread0, teste0, 0, Thread::Priority::NORMAL);
    // Thread::create(&thread1, teste1, 0, Thread::Priority::NORMAL);

    // Alarm::delay(1);
    Thread::Thread thread0(teste0, 0, Thread::Priority::NORMAL);
    Thread::Thread thread1(teste1, 0, Thread::Priority::NORMAL);
    Logger::log("Done!\n");
    // Semaphore::create(&semaphore, 1);
    Thread::join(&thread0);
    Thread::join(&thread1);
    return 0;
}
