#include <Alarm.hpp>
#include <IO/Debug.hpp>
#include <Thread.hpp>

extern int main(void *);
static volatile int _count = 0;
static Scheduler<Thread> _scheduler;
static Spin spin{!Spin::LOCKED};
static Spin boot{!Spin::LOCKED};

//__attribute__((naked)) void save() {
//    CPU::Context::push();
//    reinterpret_cast<Thread *>(CPU::thread())->context = CPU::Context::get();
//    __asm__ volatile("ret");
//}
//
// void Thread::dispatch() {
//    save();
//    if (running()->context == CPU::Context::get()) {
//        // spin.acquire();
//        // if (running()->state == State::READY) _scheduler.push(&running()->link);
//        Thread *next = _scheduler.chose();
//        next->state  = State::RUNNING;
//        spin.release();
//        CPU::Context::jump(next->context);
//    }
//}

__attribute__((naked)) void Thread::dispatch() {
    CPU::Context::save();
    running()->context = CPU::Context::get();
    Thread *next       = _scheduler.chose();
    next->state        = State::RUNNING;

    spin.release();
    CPU::Context::jump(next->context);
}

int Thread::idle(void *) {
    while (_count > Machine::CPUS) {
        if (!_scheduler.empty()) yield();
    }

    CPU::Interrupt::disable();
    if (CPU::core() == 0) Logger::println("*** QUARK Shutdown! ***\n");
    for (;;);
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

    spin.release();
    Memory::kfree(stack);
}

void Thread::join(Thread &thread) {
    spin.lock();

    if (&thread == running()) {
        spin.unlock();
        ERROR(true, "[Thread::join] Join itself.");
        return;
    }

    if (thread.joining != nullptr) {
        spin.unlock();
        ERROR(true, "[Thread::join] Already joined.");
        return;
    }

    if (thread.state == State::FINISHED) {
        spin.unlock();
        return;
    }

    Thread *previous = running();
    previous->state  = State::WAITING;
    thread.joining   = previous;

    dispatch();
    CPU::Interrupt::enable();
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
    if (CPU::core() == 0) {
        new (Memory::SYSTEM) Thread(main, 0, Criterion::NORMAL);
        while (_count < Machine::CPUS + 1);
        boot.release();
    }
    boot.lock();
    boot.release();
}

void Thread::run() {
    spin.acquire();
    Thread *first = _scheduler.chose();
    spin.release();

    first->state = State::RUNNING;
    CPU::Context::jump(first->context);
}

void Thread::reschedule() {
    auto previous = running();

    spin.acquire();

    if (_scheduler.empty()) {
        spin.release();
        return;
    }

    previous->state = State::READY;
    _scheduler.insert(&previous->link);

    dispatch();
}

void Thread::yield() {
    CPU::Interrupt::disable();
    reschedule();
    CPU::Interrupt::enable();
}

void Thread::lock() { spin.lock(); }
void Thread::unlock() { spin.unlock(); }

void Thread::sleep(Queue &waiting) {
    auto previous     = running();
    previous->state   = State::WAITING;
    previous->waiting = &waiting;
    waiting.insert(&previous->link);
    dispatch();
}

void Thread::wakeup(Queue &waiting) {
    ERROR(waiting.empty(), "[Thread::wakeup] Empty queue.");
    Element *awake        = waiting.next();
    awake->value->state   = State::READY;
    awake->value->waiting = nullptr;
    _scheduler.insert(awake);
}

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
