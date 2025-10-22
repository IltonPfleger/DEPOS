#include <Alarm.hpp>
#include <IO/Console.hpp>
#include <Memory.hpp>
#include <Semaphore.hpp>
#include <Thread.hpp>

static constexpr int FILOSOFOS  = 5;
static constexpr int ITERATIONS = 2;

Thread *threads[FILOSOFOS];
Semaphore *garfos[FILOSOFOS];
Semaphore *lock;

int filosofo(void *arg) {
    int id = (int)(long long)arg;
    int p1 = (id < FILOSOFOS - 1) ? id : 0;
    int p2 = (id < FILOSOFOS - 1) ? (id + 1) : FILOSOFOS - 1;
    int i  = ITERATIONS;
    while (i--) {
        lock->p();
        Console::println("Filósofo %d está pensando\n", id);
        lock->v();
        Alarm::usleep(10000);

        garfos[p1]->p();
        garfos[p2]->p();

        lock->p();
        Console::println("Filósofo %d está comendo\n", id);
        lock->v();
        Alarm::usleep(10000);

        garfos[p1]->v();
        garfos[p2]->v();
    }
    return 0;
}

int main(void *) {
    Console::println("Application: \n");

    lock = new (Memory::APPLICATION) Semaphore(1);

    lock->p();
    for (int i = 0; i < FILOSOFOS; i++) {
        garfos[i]  = new (Memory::APPLICATION) Semaphore(1);
        threads[i] = new (Memory::APPLICATION) Thread(filosofo, (void *)(long long)i, Thread::NORMAL);
    }
    lock->v();

    for (int i = 0; i < FILOSOFOS; i++) Thread::join(threads[i]);

    for (int i = 0; i < FILOSOFOS; i++) {
        delete threads[i];
        delete garfos[i];
    }

    Console::println("Done!\n");
    return 0;
}
