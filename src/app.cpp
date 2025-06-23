#include <io/logger.hpp>
#include <thread.hpp>

#define ITERATIONS 10

int teste0(void *) {
    int i = 0;
    while (i < ITERATIONS) {
        Logger::log("Thread0 %d\n", i);
        Thread::yield();
        i++;
    }
    return 0;
}

int teste1(void *) {
    int i = 0;
    while (i < ITERATIONS) {
        Logger::log("Thread1 %d\n", i);
        Thread::yield();
        i++;
    }
    return 0;
}

int main(void *) {
    Thread thread0;
    Thread thread1;

    Thread::create(&thread0, teste0, Thread::Priority::NORMAL);
    Thread::create(&thread1, teste1, Thread::Priority::NORMAL);

    Thread::join(&thread0);
    Logger::log("APP: thread0\n");
    Thread::join(&thread1);
    Logger::log("APP: thread1\n");

    return 0;
}
