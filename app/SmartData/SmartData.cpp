#include <Alarm.hpp>
#include <Semaphore.hpp>
#include <architecture/CPU.hpp>
#include <architecture/Timer.hpp>
#include <machine/Machine.hpp>
#include <main_traits.h>
#include <smartdata.h>
#include <system/traits.h>
#include <utility/debug.h>
#include <utils/Console.hpp>

typedef void (*constructor_t)();

extern constructor_t __init_array_start[];
extern constructor_t __init_array_end[];

void call_global_constructors() {
    for (constructor_t *ctor = __init_array_start; ctor != __init_array_end; ++ctor) {
        (*ctor)();
    }
}

static constexpr int Iterations = 10;

void *sink(void *) {
    Antigravity_Proxy a(
        Antigravity::Region(0, 0, 0, 100, Antigravity::now(), Antigravity::now() + (Iterations + 5) * 1000000),
        10000000);

    for (unsigned int i = 0; i < Iterations + 5; i++) {
        DEPOS::Console::cout << "Received a=" << i << DEPOS::Console::endl;
        DEPOS::Alarm::udelay(1000000);
    }

    return nullptr;
}

int main(int, char *[]) {
    call_global_constructors();

    TSTP::init();

    Antigravity a(0, 1000000, SmartData::ADVERTISED);

    new Thread(sink);

    unsigned int i = 0;
    while (1) {
        a = i++;
        DEPOS::Console::cout << "Send a=" << i << DEPOS::Console::endl;
        DEPOS::Alarm::udelay(1000000);
    }
}
