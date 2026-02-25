namespace DEPOS {
#include <Alarm.hpp>
#include <Semaphore.hpp>
#include <abstractions/CPU.hpp>
#include <architecture/Timer.hpp>
#include <utils/Console.hpp>
} // namespace DEPOS

#include <main_traits.h>
#include <smartdata.h>
#include <system/traits.h>
#include <utility/debug.h>

int main(int, char *[]) {
    Antigravity a(0, 1000000, SmartData::ADVERTISED);

    for (unsigned int i = 0; i < 10000; i++) {
        a = i;
        DEPOS::Console::cout << "a=" << (unsigned int)a << DEPOS::Console::endl;
        DEPOS::Alarm::udelay(1000000);
    }

    DEPOS::Console::cout << "SmartData:\n ";
}
