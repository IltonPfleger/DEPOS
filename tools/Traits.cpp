#include <Traits.hpp>
#include <iostream>

using namespace std;
using namespace DEPOS;

int main() {
    cout << "\nMULTITASK := " << Traits<Kernel>::Multitask;
    cout << "\nMACHINE_NAME := " << Traits<Machine>::NAME;
    cout << "\nARCH := " << Traits<CPU>::Architecture;
    cout << "\nCPUS := " << Traits<CPU>::Count;
    cout << "\nMEMORY_SIZE := " << Traits<Memory>::Size;
    cout << "\nPAGE_SIZE := " << Traits<Memory>::PageSize;
    cout << "\nRAM_START := 0x" << hex << Traits<MemoryMap>::RamStart;
    cout << "\nRAM_END := 0x" << hex << Traits<MemoryMap>::RamEnd;
    cout << "\nKERNEL_ADDR := 0x" << hex << Traits<MemoryMap>::KernelAddr;
    cout << "\nAPPLICATION_ADDR := 0x" << hex << Traits<Application>::Addr;
}
