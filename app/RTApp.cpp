#include <IO/Logger.hpp>
#include <Memory.hpp>
#include <Semaphore.hpp>
#include <Thread.hpp>

int teste(void*) {
    Logger::println("teste\n");
    return 0;
}

int main() {
    Logger::println("Real Time Application: \n");

    RThread thread(teste, 0, Thread::NORMAL);

    Logger::println("Done!\n");
    return 0;
}
