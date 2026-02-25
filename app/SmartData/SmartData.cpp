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

int main(int, char *[]) { Console::cout << "SmartData:\n "; }
