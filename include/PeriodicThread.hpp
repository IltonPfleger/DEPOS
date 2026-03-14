#pragma once

#include <Alarm.hpp>
#include <Thread.hpp>
#include <architecture/Timer.hpp>

namespace DEPOS {

class PeriodicThread : Thread {
    using Thread::Argument;
    using Thread::Criterion;
    using Thread::Function;

  public:
    PeriodicThread(Function f, Argument a, Criterion c, Microsecond t)
        : Thread(f, a, c),
          m_period(t),
          m_next(Timer::now() + m_period) {}

    static void wait() {
        Thread *t         = Thread::running();
        PeriodicThread *p = static_cast<PeriodicThread *>(t);

        p->m_next += p->m_period;
        if (Timer::now() >= p->m_next) ERROR(true);
        Microsecond next = p->m_next;
        Alarm::until(next);
    }

  private:
    Microsecond m_period;
    Microsecond m_next;
};

} // namespace DEPOS
