#include <Traits.hpp>
#include <machine/Machine.hpp>
#include <utils/Console.hpp>
#include <utils/Observer.hpp>

using namespace DEPOS;

class Waiter : public Observer<const unsigned char *, size_t> {
  public:
    Waiter()
        : m_done(false) {}

    void update(const unsigned char *buffer, size_t size) override {
        Console::cout << "Pressed Key: " << buffer[size - 1] << '\n';
        m_done = true;
    }

    volatile bool m_done;
};

int main(int, char *[]) {
    typedef Meta::GetFromTypeList<Traits<UART>::Devices, 0>::Result Device;
    TraceIn();
    Waiter *w = new Waiter();
    Device::instance()->attach(w);
    Console::cout << "Hello World!\n";
    Console::cout << "Press a Key to Exit...\n";
    while (!w->m_done)
        ;
    delete w;
    TraceOut();
    return 0;
}
