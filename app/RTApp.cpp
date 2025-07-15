#include <IO/Logger.hpp>
#include <Memory.hpp>
#include <Semaphore.hpp>
#include <Thread.hpp>

static constexpr int N = 10;
RT_Thread* thread[N];

int teste(void* id) {
    while (1) {
        Logger::println("THREAD: %d\n", (long long)id);
        RT_Thread::wait_next();
    }
    return 0;
}

int main() {
    Logger::println("Real Time Application: \n");

    for (int i = 0; i < N; i++) thread[i] = new (Memory::APPLICATION) RT_Thread(teste, (void*)(long long)i, 100'000);

    for (int i = 0; i < N; i++) {
        Thread::join(thread[i]);
        delete thread[i];
    }

    Logger::println("Application Done!\n");
    return 0;
}
