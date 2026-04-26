#include <architecture/Timer.hpp>
#include <kernel/Thread.hpp>
#include <shared/console/Console.hpp>
#include <shared/synchronization/Semaphore.hpp>

using namespace DEPOS;

static constexpr int Number     = 100;
static constexpr int Iterations = 100;

Semaphore *forks[Number];
Semaphore *console;
Semaphore *finish;

void *philosopher(void *p) {
    unsigned int id = (unsigned int)(unsigned long)p;

    int iterations = Iterations;
    int left       = id;
    int right      = (id + 1) % Number;

    while (iterations--) {
        console->p();

        Console::println("<", CPU::id(), "> Filósofo ", id, " está pensando!");

        console->v();

        if (id == Number - 1) {
            forks[right]->p();
            forks[left]->p();
        } else {
            forks[left]->p();
            forks[right]->p();
        }

        console->p();

        Console::println("<", CPU::id(), "> Filósofo ", id, " está comendo!");

        console->v();

        forks[right]->v();
        forks[left]->v();
    }

    return 0;
}

int main(int, char *[]) {
    TraceIn();

    Thread *threads[Number];

    console = new Semaphore(0);
    finish  = new Semaphore(0);

    for (long i = 0; i < Number; i++) {
        forks[i] = new Semaphore(1);
    }

    for (long i = 0; i < Number; i++) {
        threads[i] = new Thread(philosopher, (void *)i, Thread::Criterion::NORMAL);
    }

    console->v();

    for (long i = 0; i < Number; i++) {
        Thread::join(threads[i]);
    }

    for (long i = 0; i < Number; i++) {
        delete threads[i];
    }

    TraceOut();
}
