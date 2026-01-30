#include <Traits.hpp>
#include <iostream>

using namespace std;

int main() {
    cout << "MACHINE_NAME := " << Traits<Machine>::NAME << endl;
    cout << "CPUS_COUNT := " << Traits<CPUS>::COUNT << endl;
    cout << "MEMORY_SIZE := " << Traits<Memory>::Size << endl;
    cout << "PAGE_SIZE := " << Traits<Memory>::PageSize << endl;
    cout << "RAM_START := 0x" << hex << Traits<MemoryMap>::RamStart << endl;
    cout << "RAM_END := 0x" << hex << Traits<MemoryMap>::RamEnd << endl;
    cout << "SYSTEM_ADDR := 0x" << hex << Traits<MemoryMap>::SystemAddr << endl;
    cout << "APPLICATION_ADDR := 0x" << hex << Traits<Application>::Addr << endl;
}
