#include <Alarm.hpp>
#include <IO/Logger.hpp>
#include <Memory.hpp>
#include <Semaphore.hpp>
#include <Thread.hpp>

static constexpr int FILOSOFOS  = 5;
static constexpr int ITERATIONS = 1;

Thread *threads[FILOSOFOS];
Semaphore *garfos[FILOSOFOS];

int filosofo(void *arg) {
    int id = (int)(long long)arg;
    int p1 = (id < FILOSOFOS - 1) ? id : 0;
    int p2 = (id < FILOSOFOS - 1) ? (id + 1) : FILOSOFOS - 1;
    int i  = ITERATIONS;
    while (i--) {
        Logger::log("Filósofo %d está pensando\n", id);
        Alarm::udelay(100000);

        garfos[p1]->p();
        garfos[p2]->p();

        Logger::log("Filósofo %d está comendo\n", id);
        Alarm::udelay(100000);

        garfos[p1]->v();
        garfos[p2]->v();
    }
    return 0;
}

int main(void *) {
    Logger::log("Application: \n");

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
