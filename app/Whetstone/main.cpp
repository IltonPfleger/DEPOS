#include <Semaphore.hpp>
#include <Thread.hpp>
#include <utility/Console.hpp>
#include <utility/Delay.hpp>

using namespace DEPOS;

static constexpr int Number     = 100;
static constexpr int Iterations = 100;

Semaphore *console;
Semaphore *start;
Thread *threads[Number];

void *worker(void *) {
    double value     = 0;
    double increment = 0.1;

    start->p();

    for (int i = 0; i < Iterations; i++) {
        value += increment;
    };

    console->p();
    Console::println("<", CPU::id(), ">", " Result: ", value, "!");
    console->v();

    return nullptr;
}

int main(int, char *[]) {
    console = new Semaphore(1);
    start   = new Semaphore(0);

    for (int i = 0; i < Number; i++)
        threads[i] = new Thread(worker);

    for (int i = 0; i < Number; i++)
        start->v();

    for (int i = 0; i < Number; i++)
        Thread::join(*threads[i]);

    for (int i = 0; i < Number; i++)
        delete threads[i];

    return 0;
}
