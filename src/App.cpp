#include <Alarm.hpp>
#include <IO/Logger.hpp>
#include <Memory.hpp>
#include <Semaphore.hpp>
#include <Thread.hpp>

#define FILOSOFOS 2
#define ITERATIONS 5

Thread *threads[FILOSOFOS];
Semaphore *garfos[FILOSOFOS];
Semaphore *mutex;

int filosofo(void *arg) {
    int id       = (int)(long)arg;
    int esquerda = id;
    int direita  = (id + 1) % FILOSOFOS;
    int i        = ITERATIONS;
    while (i--) {
        Logger::log("Filósofo %d está pensando\n", id);
        Alarm::delay(1);

        mutex->p();
        garfos[esquerda]->p();
        garfos[direita]->p();
        mutex->v();

        Logger::log("Filósofo %d está comendo\n", id);
        Alarm::delay(1);

        garfos[direita]->v();
        garfos[esquerda]->v();
    }
    return 0;
}

int main(void *) {
    Logger::log("Application: \n");

    mutex = new (Memory::APPLICATION) Semaphore(1);
    for (int i = 0; i < FILOSOFOS; i++) {
        garfos[i] = new (Memory::APPLICATION) Semaphore(1);
    }

    for (int i = 0; i < FILOSOFOS; i++) {
        threads[i] = new (Memory::APPLICATION) Thread(filosofo, (void *)(long long)i, Thread::Priority::NORMAL);
    }

    for (int i = 0; i < FILOSOFOS; i++) {
        Thread::join(threads[i]);
        delete threads[i];
        delete garfos[i];
    }

    Logger::log("Done!\n");
    return 0;
}
