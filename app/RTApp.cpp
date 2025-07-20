#include <Alarm.hpp>
#include <IO/Logger.hpp>
#include <Memory.hpp>
#include <Semaphore.hpp>
#include <Thread.hpp>

static constexpr int N = 4;
RT_Thread* thread[N];

int teste(void* id) {
    Logger::println("THREAD %d\n", (long long)id);
    return 0;
}

int main() {
    Logger::println("Real Time Application: \n");

    auto now  = Alarm::utime() + 5'000;
    thread[0] = new (Memory::APPLICATION) RT_Thread(teste, (void*)(long long)1, 2'000, 2'000, 0, now);
    thread[1] = new (Memory::APPLICATION) RT_Thread(teste, (void*)(long long)2, 10'000, 2'000, 0, now + 1'000);
    thread[2] = new (Memory::APPLICATION) RT_Thread(teste, (void*)(long long)3, 10'000, 2'000, 0, now + 2'000);
    thread[3] = new (Memory::APPLICATION) RT_Thread(teste, (void*)(long long)4, 10'000, 2'000, 0, now + 3'000);
    // for (int i = 0; i < N; i++)
    //     thread[i] = new (Memory::APPLICATION) RT_Thread(teste, (void*)(long long)i, 1'000, 1'000, 100, now + i *
    //     1'000);

    for (int i = 0; i < N; i++) {
        Thread::join(thread[i]);
        delete thread[i];
    }

    Logger::println("Application Done!\n");
    return 0;
}
