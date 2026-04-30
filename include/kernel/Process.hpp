#ifndef __DEPOS_KERNEL_PROCESS_HEADER__
#define __DEPOS_KERNEL_PROCESS_HEADER__

#include <archtecture/MMU.hpp>
#include <kernel/Thread.hpp>

namespace DEPOS {

class Process {
    using PageTable = MMU::PageTable;

  public:
  private:
    Thread *m_thread;
    PageTable m_pt;
};

} // namespace DEPOS

#endif
