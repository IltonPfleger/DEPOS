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

static const unsigned int ITERATIONS = 10;

int main(int, char *[]) {
    // typedef DEPOS::Meta::GetFromTypeList<DEPOS::Traits<DEPOS::Ethernet>::Devices, 0>::Result Device;

    // Device::init();

    call_global_constructors();

    TSTP::init();

    Antigravity a(0, 1000000, SmartData::ADVERTISED);

    unsigned int i = 0;
    while (1) {
        a = i++;
        DEPOS::Console::cout << "a=" << i << DEPOS::Console::endl;
        DEPOS::Alarm::udelay(1000000);
    }
}
