#include <Semaphore.hpp>
#include <Thread.hpp>
#include <utils/Console.hpp>
#include <utils/Debug.hpp>

static constexpr int Number = 10;
static constexpr int Iterations = 5;
Semaphore *forks[Number];
Semaphore *console;

int philosopher(void *p) {
    unsigned int id = (unsigned int)(unsigned long)p;

    int iterations = Iterations;
    int left = id;
    int right = (id + 1) % Number;

    while (iterations--) {
        console->p();

        Console::print("Filósofo ");
        Console::print(id);
        Console::print(" esta pensando!\n");

        console->v();

        forks[left]->p();
        forks[right]->p();

        console->p();

        Console::print("Filósofo ");
        Console::print(id);
        Console::print(" está comendo!\n");

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

    for (long i = 0; i < Number; i++) {
        forks[i] = new Semaphore(1);
    }

    for (long i = 0; i < Number; i++) {
        threads[i] = new Thread(philosopher, (void *)i, Thread::Criterion::NORMAL);
    }

    console->v();

    for (long i = 0; i < Number; i++) {
        // Thread::join(threads[i]);
    }

    for (long i = 0; i < Number; i++) {
        delete forks[i];
        delete threads[i];
    }

    delete console;

    TraceOut();
}
