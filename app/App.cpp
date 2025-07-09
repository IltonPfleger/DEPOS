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
    int id       = (int)(long long)arg;
    int esquerda = id;
    int direita  = (id + 1) % FILOSOFOS;
    int i        = ITERATIONS;
    while (i--) {
        Logger::log("Filósofo %d está pensando\n", id);
        Alarm::udelay(100);

        if (id == FILOSOFOS - 1) {
            garfos[esquerda]->p();
            garfos[direita]->p();
        } else {
            garfos[direita]->p();
            garfos[esquerda]->p();
        }

        Logger::log("Filósofo %d está comendo\n", id);
        Alarm::udelay(100);

        garfos[direita]->v();
        garfos[esquerda]->v();
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
