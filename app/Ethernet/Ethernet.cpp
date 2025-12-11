#include <drivers/ethernet/dwmac.hpp>
#include <drivers/phy/YT8231C.hpp>
#include <machine/Machine.hpp>
#include <utils/Console.hpp>
#include <utils/Debug.hpp>

int main(int, char *[]) {
    TraceIn();
    // YT8231C::init();
    Ethernet::init();
    TraceOut();
    return 0;
}
