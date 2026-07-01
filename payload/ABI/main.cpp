#include <Traits.hpp>
#include <abi/Thread.hpp>
#include <machine/Machine.hpp>
#include <utility/Console.hpp>

using namespace QUARK::ABI;

void *function(void *) {
    QUARK::Console::println("Function!");
    return nullptr;
}

int main(int, char *[]) {
    QUARK::Console::println("Hello World!");
    Thread thread(function, nullptr);
    thread.join();
    return 0;
}
