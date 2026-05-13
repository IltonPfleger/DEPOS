#include <Semaphore.hpp>
#include <Thread.hpp>
#include <libraries/libc/string.h>
#include <utility/Console.hpp>
#include <utility/Debug.hpp>

using namespace DEPOS;

static constexpr int Number     = 100;
static constexpr int Iterations = 100;

void *allocators(void *) {
    int iterations = Iterations;
    int size       = (Traits<Memory>::Size - Traits<Memory>::Size / 8) / (Number * 2);

    while (iterations--) {
        unsigned char *buffer = new unsigned char[size];
        memset(buffer, 0, size);
        delete[] buffer;
    }

    return 0;
}

int main(int, char *[]) {
    TraceIn();

    Thread *threads[Number];

    for (long i = 0; i < Number; i++) {
        threads[i] = new Thread(allocators);
    }

    for (long i = 0; i < Number; i++) {
        delete threads[i];
    }

    TraceOut();
}
