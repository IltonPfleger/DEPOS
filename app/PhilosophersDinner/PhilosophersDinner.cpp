#include <Semaphore.hpp>
#include <Thread.hpp>
#include <utils/Console.hpp>
#include <utils/Debug.hpp>

static constexpr int Number = 100;
static constexpr int Iterations = 100;
Semaphore *forks[Number];
Semaphore *console;

int philosopher(void *p) {
    unsigned int id = (unsigned int)(unsigned long)p;

    int iterations = Iterations;
    int left = id;
    int right = (id + 1) % Number;

    while (iterations--) {
        console->p();

        Console::println("<%d> Filósofo %d está pensando!\n", CPU::id(), id);

        console->v();

        if (id == Number - 1) {
            forks[right]->p();
            forks[left]->p();
        } else {
            forks[left]->p();
            forks[right]->p();
        }

        console->p();

        Console::println("<%d> Filósofo %d está comendo!\n", CPU::id(), id);

        console->v();

        forks[right]->v();
        forks[left]->v();
    }

    return 0;
}

int main(int, char *[]) {
    TraceIn();

    Thread *threads[Number];
    (void)threads;

    console = new Semaphore(0);

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

    // for (long i = 0; i < Number; i++) {
    //     delete forks[i];
    //     delete threads[i];
    // }

    // delete console;

    TraceOut();
}
