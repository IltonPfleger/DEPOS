#pragma once

#include <utils/Observer.hpp>

template <typename NIC> class ARP : public Observer<unsigned char *> {
    ARP() {
        m_nic = NIC::instance();
        m_nic->attach(this);
    }
    ~ARP() { m_nic->dettach(this); }

  private:
    NIC *m_nic;
};
