#include <Alarm.hpp>
#include <Semaphore.hpp>
#include <abstractions/CPU.hpp>
#include <architecture/Timer.hpp>
#include <utils/Console.hpp>

#define _UTIL EPOS
#define _SYS EPOS
namespace EPOS {
#include <main_traits.h>
#include <system/traits.h>
#include <utility/debug.h>
/**/
#include <smartdata.h>
} // namespace EPOS

int main(int, char *[]) {
    EPOS::Antigravity a(0, 1000000, EPOS::SmartData::ADVERTISED);

    for (unsigned int i = 0; i < 10000; i++) {
        a = i;
        Console::cout << "a=" << (unsigned int)a << Console::endl;
        Alarm::udelay(1000000);
    }

    Console::cout << "SmartData:\n ";
}
