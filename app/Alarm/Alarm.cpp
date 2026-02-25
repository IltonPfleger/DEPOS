#include <Alarm.hpp>

int main(int, char *[]) {
    Console::cout << "Alarm::udelay()...";
    Alarm::udelay(1'000'000);
    Console::cout << Console::endl;
};
