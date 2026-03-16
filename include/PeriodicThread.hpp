#pragma once

#include <Alarm.hpp>
#include <Thread.hpp>
#include <architecture/Timer.hpp>

namespace DEPOS {

template <unsigned int N> class Data {
  public:
    Data()
        : m_index(0) {}

    void collect(Microsecond ts) {
        if (m_index < N) m_buffer[m_index++] = ts;
        if (m_index >= N) {
            CPU::Interruptions::disable();
            Console::cout << Console::endl;
            Console::cout << "*** Start ***" << Console::endl;
            flush();
            Console::cout << "*** End ***" << Console::endl;
            while (1)
                ;
        }
    }

    void flush() {
        for (unsigned int i = 0; i < m_index; i++) {
            Console::cout << m_buffer[i] << Console::endl;
        }
        m_index = 0;
    }

  private:
    Microsecond m_buffer[N];
    unsigned int m_index;
};

class PeriodicThread : Semaphore, Thread {
    using Thread::Argument;
    using Thread::Criterion;
    using Thread::Function;

    struct Arguments {
        Function m_function;
        Argument m_argument;
    };

    static void *dispatch(void *argument) {
        PeriodicThread *self = static_cast<PeriodicThread *>(argument);
        self->p();
        self->m_next = Timer::now() + self->m_period;
        self->m_arguments.m_function(self->m_arguments.m_argument);
        return 0;
    }

  public:
    PeriodicThread(Function f, Argument a, Criterion c, Microsecond t)
        : Semaphore(0),
          Thread(dispatch, this, c),
          m_arguments(f, a),
          m_period(t) {
        this->v();
    }

    static void wait() {
        PeriodicThread *self = static_cast<PeriodicThread *>(Thread::running());
        Alarm::at(self->m_next);
        self->m_next += self->m_period;
        self->m_data.collect(Timer::now());
    }

  private:
    Data<10'000> m_data;
    Arguments m_arguments;
    Microsecond m_period;
    Microsecond m_next;
};

} // namespace DEPOS
