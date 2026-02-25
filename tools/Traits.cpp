#include <Traits.hpp>
#include <iostream>

using namespace std;

int main() {
    cout << "MACHINE_NAME := " << Traits<Machine>::NAME << endl;
    cout << "ARCH := " << Traits<CPU>::Architecture << endl;
    cout << "CPUS := " << Traits<CPU>::Count << endl;
    cout << "MEMORY_SIZE := " << Traits<Memory>::Size << endl;
    cout << "PAGE_SIZE := " << Traits<Memory>::PageSize << endl;
    cout << "RAM_START := 0x" << hex << Traits<MemoryMap>::RamStart << endl;
    cout << "RAM_END := 0x" << hex << Traits<MemoryMap>::RamEnd << endl;
    cout << "KERNEL_ADDR := 0x" << hex << Traits<MemoryMap>::KernelAddr << endl;
    cout << "APPLICATION_ADDR := 0x" << hex << Traits<Application>::Addr << endl;
}
