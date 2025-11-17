#include <Machine.hpp>
#include <utils/Console.hpp>

void Console::init() { Machine::IO::init(); }
void Console::put(char c) { Machine::IO::put(c); }
