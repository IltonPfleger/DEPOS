#include <io/logger.hpp>
#include <thread.hpp>

#define ITERATIONS 10
#define SLEEP 10000000

int teste0(void *) {
    int i = 0;
    while (i < ITERATIONS) {
        Logger::log("Thread0 %d\n", i);
        volatile int j = SLEEP;
        while (j--);
        i++;
    }
    return 0;
}

int teste1(void *) {
    int i = 0;
    while (i < ITERATIONS) {
        Logger::log("Thread1 %d\n", i);
        volatile int j = SLEEP;
        while (j--);
        i++;
    }
    return 0;
}

int main(void *) {
    Logger::log("APP:\n");
    Thread thread0;
    Thread thread1;
    Thread::create(&thread0, teste0, 0, Thread::Priority::NORMAL);
    Thread::create(&thread1, teste1, 0, Thread::Priority::NORMAL);
    Thread::join(&thread0);
    Thread::join(&thread1);
    return 0;
}
