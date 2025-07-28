#include <Alarm.hpp>
#include <IO/Debug.hpp>
#include <Thread.hpp>

extern int main(void *);
static Spin spin;

__attribute__((naked)) static void dispatch() {
    CPU::Context::push();
    Thread::running()->context = CPU::Context::get();
    Thread *next               = Thread::_scheduler.chose();
    spin.release();

    next->state = Thread::State::RUNNING;
    CPU::Context::jump(next->context);
}

int Thread::idle(void *) {
    // while (_count > Machine::CPUS) {
    //  if (!_scheduler.empty()) {
    //      spin.lock();
    //      auto previous   = running();
    //      previous->state = State::READY;
    //      _scheduler.insert(&previous->link);
    //      dispatch();
    //  }
    //}

    // CPU::Interrupt::disable();
    // if (CPU::core() == 0) Logger::println("*** QUARK Shutdown! ***\n");
    for (;;);  // CPU::idle();
    return 0;
}

Thread::Thread(Function f, Argument a, Criterion c)
    : stack(reinterpret_cast<char *>(Memory::kmalloc())),
      context(new(stack + Traits::Memory::Page::SIZE - sizeof(CPU::Context)) CPU::Context(f, exit, this, a)),
      state(State::READY),
      joining(0),
      criterion(c),
      link(Element(this, c.priority())),
      waiting(0) {
    spin.lock();
    _scheduler.insert(&link);
    _count = _count + 1;
    spin.unlock();
}

Thread::~Thread() {
    spin.lock();
    switch (state) {
        case (State::READY):
            _scheduler.remove(&link);
            _count = _count - 1;
            break;
        case (State::WAITING):
            waiting->remove(&link);
            _count = _count - 1;
            break;
        default:
            break;
    }

    if (joining) {
        joining->state = State::READY;
        _scheduler.insert(&joining->link);
    }

    Memory::kfree(stack);
    spin.unlock();
}

void Thread::join(Thread *thread) {
    ERROR(thread == nullptr, "[Thread::join] Invalid thread.");

    spin.lock();

    if (thread == running()) {
        spin.unlock();
        ERROR(true, "[Thread::join] Join itself.");
        return;
    }

    if (thread->joining != nullptr) {
        spin.unlock();
        ERROR(true, "[Thread::join] Already joined.");
        return;
    }

    if (thread->state == State::FINISHED) {
        spin.unlock();
        return;
    }

    Thread *previous = running();
    previous->state  = State::WAITING;
    thread->joining  = previous;

    dispatch();
}

void Thread::exit() {
    auto previous = running();

    spin.lock();
    previous->state = State::FINISHED;

    if (previous->joining) {
        previous->joining->state = State::READY;
        _scheduler.insert(&previous->joining->link);
        previous->joining = 0;
    }

    _count = _count - 1;
    dispatch();
}

void Thread::init() {
    new (Memory::SYSTEM) Thread(idle, 0, Criterion::IDLE);
    if (!CPU::core()) new (Memory::SYSTEM) Thread(main, 0, Criterion::NORMAL);

    spin.lock();
    Thread *first = _scheduler.chose();
    spin.release();

    first->state = State::RUNNING;
    CPU::thread(first);
}

void Thread::run() { CPU::Context::jump(running()->context); }

void Thread::reschedule() {
    auto previous = running();

    spin.acquire();

    if (_scheduler.empty()) {
        spin.release();
        return;
    }

    previous->state = State::READY;
    _scheduler.insert(&previous->link);
    Thread *next = Thread::_scheduler.chose();

    spin.release();

    next->state = Thread::State::RUNNING;
    CPU::thread(next);
}

// void Thread::sleep(Queue *waiting) {
//     lock();
//
//     Thread *previous  = const_cast<Thread *>(running());
//     previous->state   = State::WAITING;
//     previous->waiting = waiting;
//     waiting->insert(previous->link);
//     Thread *next = _scheduler.chose();
//     dispatch(previous, next);
// }
//
// void Thread::wakeup(Queue *waiting) {
//     lock();
//
//     Element *awake = waiting->next();
//     ERROR(awake == nullptr, "[Thread::wakeup] Empty queue.");
//     awake->value->state   = State::READY;
//     awake->value->waiting = nullptr;
//     _scheduler.insert(awake);
//     _lock.unlock();
// }

// int entry(void *arg) {
//     RT_Thread *current = const_cast<RT_Thread *>(static_cast<volatile RT_Thread *>(Thread::running()));
//     auto now           = Alarm::utime();
//     if (now < current->start) Alarm::usleep(current->start - now);
//
//     while (1) {
//         current->function(arg);
//         now = Alarm::utime();
//
//         int miss = (now - current->start) - current->deadline;
//         ERROR(miss > 0, "Missed deadline: %dμ\n", miss);
//
//         current->start += current->period;
//         Alarm::usleep(current->start - now);
//     }
//     return 0;
// }
//
// RT_Thread::RT_Thread(Function f, Argument a, Microsecond d, Microsecond p, Microsecond c, Microsecond s)
//     : Thread(entry, a, Criterion(d, p, c)), function(f), deadline(d), period(p), duration(c), start(s) {}
