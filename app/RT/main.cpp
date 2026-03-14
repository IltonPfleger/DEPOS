#include <Thread.hpp>

using namespace DEPOS;

Thread::Return init(Thread::Argument) {
    while (1)
        ;
    return Thread::Return{};
}

int main() { new Thread(init, 0); }
