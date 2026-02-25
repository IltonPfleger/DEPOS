#include <Alarm.hpp>
#include <Semaphore.hpp>
#include <architecture/CPU.hpp>
#include <architecture/Timer.hpp>
#include <main_traits.h>
#include <smartdata.h>
#include <system/traits.h>
#include <utility/debug.h>
#include <utils/Console.hpp>

int main(int, char *[]) {
    Antigravity a(0, 1000000, SmartData::ADVERTISED);

    while (1)
        ;

    // for (unsigned int i = 0; i < 10000; i++) {
    //     a = i;
    //     DEPOS::Console::cout << "a=" << (unsigned int)a << DEPOS::Console::endl;
    //     DEPOS::Alarm::udelay(1000000);
    // }

    DEPOS::Console::cout << "SmartData:\n ";
}
