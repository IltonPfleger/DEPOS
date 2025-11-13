#include <Alarm.hpp>
#include <IO/Debug.hpp>
#include <Spin.hpp>
#include <Thread.hpp>
#include <memory/Heap.hpp>
#include <memory/Memory.hpp>
#include <memory/Segment.hpp>
#include <proxys/Syscall.hpp>

static volatile int _count = 0;
static Scheduler<Thread> _scheduler;
static Spin _lock;

Thread *Thread::running() { return _scheduler.current(); }

void Thread::dispatch(Thread *previous, Thread *next, Spin *lock) {
    next->state = State::RUNNING;
    lock->release();
    if (next != previous) {
        // if constexpr (Traits::System::MULTITASK) {
        //     // next->task_->attach(previous->stack_);
        //     // next->task_->load();
        // }

        CPU::Atomic::wait(next->context_);
        CPU::Context::swtch(const_cast<CPU::Context **>(&previous->context_), CPU::Atomic::clear(next->context_));
    }
}

int Thread::idle(void *) {
    while (_count > Traits::Machine::CPUS) {
        if (!_scheduler.empty()) yield();
    }

    CPU::Interrupt::disable();
    if (CPU::core() == Traits::Machine::BSP) Console::println("*** Shutdown! ***\n");
    for (;;);
    return 0;
}

Thread::Thread(Function f, Argument a, Criterion c)
    : stack_(Segment(Traits::Memory::Page::SIZE)),
      state(State::READY),
      joining(0),
      criterion(c),
      waiting(0),
      link(Element(this, c())) {
    TraceIn(this);

    // if constexpr (Traits::System::MULTITASK) {
    //     context_ = new (stack_.end() - sizeof(CPU::Context)) CPU::Context(f, a, Syscall::call<Thread::exit>);
    //     task_    = t;
    //     if (!task_) {
    //         task_ = new (Heap::SYSTEM) Task();
    //     }
    //     //task_->attach(stack_);
    // } else {
    context_ = new (stack_.end() - sizeof(CPU::Context)) CPU::Context(f, a, exit);
    // }
    _lock.lock();
    _scheduler.insert(&link);
    _count = _count + 1;
    _lock.unlock();
    TraceOut();
}

// Thread::Thread(Task *t, Function f, Argument a, Criterion c) : task_(t), Thread(f, a, c) {}

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
//         _scheduler.insert(&joining->link);
//     }
//
//     _lock.release();
//     Memory::kfree(stack);
// }

void Thread::join(Thread &thread) {
    auto previous = running();
    ERROR(&thread == previous, "[Thread::join] Join itself.");
    ERROR(thread.joining, "[Thread::join] Already joined.");

    _lock.lock();
    if (thread.state == State::FINISHED) {
        _lock.unlock();
        return;
    }

    previous->state = State::WAITING;
    thread.joining  = previous;

    dispatch(previous, _scheduler.pop(), &_lock);
    CPU::Interrupt::enable();
}

void Thread::exit() {
    _lock.lock();
    TraceIn();

    auto previous   = running();
    previous->state = State::FINISHED;

    if (previous->joining) {
        previous->joining->state = State::READY;
        _scheduler.insert(&previous->joining->link);
        previous->joining = 0;
    }

    _count = _count - 1;
    dispatch(previous, _scheduler.pop(), &_lock);
}

void Thread::init() {
    TraceIn();
    for (int i = 0; i < Traits::Machine::CPUS; ++i) new (Heap::SYSTEM) Thread(idle, 0, Criterion::IDLE);
    TraceOut()
}

void Thread::run() {
    char buffer[sizeof(Thread)];
    Thread *previous = reinterpret_cast<Thread *>(buffer);
    _lock.acquire();
    TraceIn();
    Thread *next = _scheduler.pop();
    dispatch(previous, next, &_lock);
}

void Thread::reschedule() {
    if (_scheduler.empty()) return;

    auto previous   = running();
    previous->state = State::READY;

    _lock.acquire();
    _scheduler.insert(&previous->link);
    dispatch(previous, _scheduler.pop(), &_lock);
}

void Thread::yield() {
    CPU::Interrupt::disable();
    reschedule();
    CPU::Interrupt::enable();
}

void Thread::sleep(Queue &waiting, Spin &lock) {
    auto previous     = running();
    previous->state   = State::WAITING;
    previous->waiting = &waiting;
    waiting.insert(&previous->link);

    _lock.acquire();
    auto next = _scheduler.pop();
    _lock.release();

    dispatch(previous, next, &lock);
}

void Thread::wakeup(Queue &waiting) {
    Element *awake = waiting.next();
    ERROR(!awake, "[Thread::wakeup] Empty queue.");
    awake->value->state   = State::READY;
    awake->value->waiting = nullptr;

    _lock.acquire();
    _scheduler.insert(awake);
    _lock.release();
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
