#include <Alarm.hpp>
#include <CPU.hpp>
#include <IO/Debug.hpp>
#include <Memory.hpp>
#include <Spin.hpp>
#include <Thread.hpp>

extern int main(void *);
static volatile int _count = 0;
static Scheduler<Thread> _scheduler;
static Spin _lock;

Thread *Thread::running() { return reinterpret_cast<Thread *>(CPU::thread()); }

__attribute__((naked)) void Thread::dispatch() {
    CPU::Context::save();
    auto previous     = running();
    previous->context = CPU::Context::get();

    if (previous->state == State::READY) {
        _scheduler.push(&previous->link);
    }

    Thread *next = _scheduler.pop();
    next->state  = State::RUNNING;

    _lock.release();
    CPU::Context::load(next->context);
}

int Thread::idle(void *) {
    while (_count > Machine::CPUS) {
        // if (!_scheduler.empty()) yield();
    }

    CPU::Interrupt::disable();
    if (CPU::core() == 0) Logger::println("*** QUARK Shutdown! ***\n");
    for (;;);
    return 0;
}

Thread::Thread(Function f, Argument a, Criterion c)
    : stack(reinterpret_cast<char *>(Memory::kmalloc())),
      context(new(stack + Traits::Memory::Page::SIZE - sizeof(CPU::Context)) CPU::Context(f, a, exit, this)),
      state(State::READY),
      joining(0),
      criterion(c),
      link(Element(this, c.priority())),
      waiting(0) {
    _lock.lock();
    _scheduler.push(&link);
    _count = _count + 1;
    _lock.unlock();
}

// Thread::~Thread() {
//     _lock.lock();
//     switch (state) {
//         case (State::READY):
//             _scheduler.remove(&link);
//             _count = _count - 1;
//             break;
//         case (State::WAITING):
//             waiting->remove(&link);
//             _count = _count - 1;
//             break;
//         default:
//             break;
//     }
//
//     if (joining) {
//         joining->state = State::READY;
//         _scheduler.push(&joining->link);
//     }
//
//     _lock.release();
//     Memory::kfree(stack);
// }

// void Thread::join(Thread &thread) {
//     auto previous = running();
//     ERROR(&thread == previous, "[Thread::join] Join itself.");
//     ERROR(thread.joining, "[Thread::join] Already joined.");
//
//     _lock.lock();
//     if (thread.state == State::FINISHED) {
//         _lock.unlock();
//         return;
//     }
//
//     previous->state = State::WAITING;
//     thread.joining  = previous;
//
//     dispatch();
//     CPU::Interrupt::enable();
// }

void Thread::exit() {
    auto previous = running();

    _lock.lock();
    previous->state = State::FINISHED;

    if (previous->joining) {
        previous->joining->state = State::READY;
        _scheduler.push(&previous->joining->link);
        previous->joining = 0;
    }

    _count = _count - 1;
    dispatch();
}

void Thread::init() {
    for (int i = 0; i < Machine::CPUS; ++i) new (Memory::SYSTEM) Thread(idle, 0, Criterion::IDLE);
    new (Memory::SYSTEM) Thread(main, 0, Criterion::NORMAL);
    CPU::Interrupt::disable();
}

void Thread::run() {
    _lock.acquire();
    Thread *first = _scheduler.pop();
    _lock.release();

    first->state = State::RUNNING;
    CPU::Context::load(first->context);
}

void Thread::reschedule() {
    if (_scheduler.empty()) return;

    auto previous   = running();
    previous->state = State::READY;

    _lock.acquire();
    dispatch();
}

// void Thread::yield() {
//     CPU::Interrupt::disable();
//     reschedule();
//     CPU::Interrupt::enable();
// }
//
// void Thread::lock() { _lock.lock(); }
// void Thread::unlock() { _lock.unlock(); }
//
// void Thread::sleep(Queue &waiting) {
//     auto previous     = running();
//     previous->state   = State::WAITING;
//     previous->waiting = &waiting;
//     waiting.insert(&previous->link);
//     dispatch();
// }
//
// void Thread::wakeup(Queue &waiting) {
//     ERROR(waiting.empty(), "[Thread::wakeup] Empty queue.");
//     Element *awake        = waiting.next();
//     awake->value->state   = State::READY;
//     awake->value->waiting = nullptr;
//     _scheduler.push(awake);
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
