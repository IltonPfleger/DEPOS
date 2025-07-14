#include <IO/Logger.hpp>
#include <Memory.hpp>
#include <Semaphore.hpp>
#include <Thread.hpp>

int teste(void*) {
    while (1) {
        Logger::println("THREAD!\n");
        RT_Thread::wait_next();
    }
    return 0;
}

int main() {
    Logger::println("Real Time Application: \n");

    RT_Thread thread(teste, 0, 100'000);
	Thread::join(&thread);

    Logger::println("Done!\n");
    return 0;
}
