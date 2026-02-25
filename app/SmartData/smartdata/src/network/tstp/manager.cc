// EPOS Trustful Space-Time Protocol Manager Implementation

// #include <system/config.h>

#define __tstp__ 1

#ifdef __tstp__

namespace DEPOS {
#include <Alarm.hpp>
#include <Semaphore.hpp>
#include <architecture/Timer.hpp>
#include <utils/Console.hpp>
} // namespace DEPOS

#include <main_traits.h>
#include <network/tstp/tstp.h>

// __BEGIN_SYS

TSTP::Manager::~Manager() {
    db<TSTP>(TRC) << "TSTP::~Manager()" << endl;
    detach(this);
}

void TSTP::Manager::update(Data_Observed<Buffer> *obs, Buffer *buf) {
    db<TSTP>(TRC) << "TSTP::Manager::update(obs=" << obs << ",buf=" << buf << ")" << endl;
}

void TSTP::Manager::marshal(Buffer *buf) { db<TSTP>(TRC) << "TSTP::Manager::marshal(buf=" << buf << ")" << endl; }

// __END_SYS

#endif
