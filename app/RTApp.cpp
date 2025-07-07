#include <IO/Logger.hpp>
#include <Memory.hpp>
#include <Semaphore.hpp>
#include <Thread.hpp>

int teste(void*) {
    Logger::log("teste\n");
    return 0;
}

int main() {
    Logger::log("Real Time Application: \n");

    RThread thread(teste, 0, Thread::NORMAL);

    Logger::log("Done!\n");
    return 0;
}
