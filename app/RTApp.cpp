#include <Alarm.hpp>
#include <IO/Logger.hpp>
#include <Memory.hpp>
#include <Semaphore.hpp>
#include <Thread.hpp>

static constexpr int N = 2;
RT_Thread* thread[N];

int teste(void* id) {
    Logger::println("%d\n", (long long)id);
    return 0;
}

int main() {
    Logger::println("Real Time Application: \n");

    auto now = Alarm::utime() + 1'000;
    for (int i = 0; i < N; i++)
        thread[i] = new (Memory::APPLICATION) RT_Thread(teste, (void*)(long long)i, 1'000, 1'000, 100, now + i * 1'000);

    for (int i = 0; i < N; i++) {
        Thread::join(thread[i]);
        delete thread[i];
    }

    Logger::println("Application Done!\n");
    return 0;
}
