#include <Alarm.hpp>
#include <Semaphore.hpp>
#include <Thread.hpp>
#include <utility/Console.hpp>
#include <utility/Debug.hpp>
#include <utility/Delay.hpp>

using namespace QUARK;

static constexpr int Agents  = 100;
static constexpr int Buckets = 1000;

Semaphore *bucket[Buckets];
Semaphore *console;
Semaphore *finish;
Thread *threads[Agents];

void *worker(void *p) {
    unsigned int id = (unsigned int)(unsigned long)p;

    for (int i = 0; i < Buckets; i++) {
        int k = Timer::now() % Buckets;
        console->p();
        Console::println("Worker: ", id, " Bucket: ", k);
        console->v();

        bucket[k]->p();
        Thread::yield();
        bucket[k]->v();
    }

    return 0;
}

int main(int, char *[]) {
    console = new Semaphore(0);

    for (long i = 0; i < Buckets; i++)
        bucket[i] = new Semaphore(1);

    for (long i = 0; i < Agents; i++) {
        constexpr int HIGHER = Thread::Criterion::HIGHER;
        int k                = i % HIGHER;
        if (k == Thread::Criterion::IDLE) k = 1;
        threads[i] = new Thread(worker, (void *)i, k);
    }

    console->v();

    for (long i = 0; i < Agents; i++)
        delete threads[i];
}
