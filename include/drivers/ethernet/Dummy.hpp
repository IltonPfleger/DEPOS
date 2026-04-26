#pragma once

#include <network/NIC.hpp>
#include <network/ethernet/Ethernet.hpp>

namespace DEPOS {

class Dummy final : public NIC {
    Dummy() {}

  public:
    int send(const void *, size_t) override { return 0; }
    Buffer *receive() override { return nullptr; }
    void free(Buffer *) override {}

    Ethernet::Address address() { return Ethernet::Address{}; }

    static auto *instance() {
        static Dummy instance;
        return &instance;
    }
};

} // namespace DEPOS
