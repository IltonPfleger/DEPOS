#pragma once

#include <network/ethernet/ip/UDP.hpp>

template <typename NIC> class TFTP {

    TFTP() : m_udp() {}

    void receive() {}

  private:
    UDP<NIC> m_udp;
};
