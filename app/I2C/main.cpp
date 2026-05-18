#include <machine/Machine.hpp>

using namespace DEPOS;

int main(int, char *[]) {

    JH7110_DVFS_Controller dvfs;

    dvfs.set({1500000000, 1040});
    // typedef Meta::GetFromTypeList<Traits<I2C>::Devices, 0>::Result I2C_Device;
    // typedef Meta::GetFromTypeList<Traits<PMIC>::Devices, 0>::Result PMIC_Device;

    // I2C_Device i2c;
    // i2c.read(0, 0, 0, 0);

    // PMIC_Device pmic(i2c);

    // Console::println(pmic.voltage(2, 1040));
    // Console::println(pmic.voltage(2));

    //    using Device = IPMSCANFD<CAN0>;
    //
    //    auto device = Device::instance();
    //
    //    while (1) {
    //        unsigned char data[] = {0, 1, 2, 4};
    //        device.send(0, data, 4);
    //
    //        NetworkBuffer *raw = device.doReceive();
    //
    //        if (raw) {
    //            CAN::Buffer *buffer = static_cast<CAN::Buffer *>(raw);
    //            Console::println("ID: ", Console::Hex(buffer->id()));
    //            Console::println("Length: ", buffer->length());
    //            Console::println("Data: ", Console::Hex(*buffer->data<uintptr_t *>()));
    //            device.release(raw);
    //        }
    //    }
    return 0;
}
