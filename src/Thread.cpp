#include <Alarm.hpp>
#include <IO/Debug.hpp>
#include <Spin.hpp>
#include <Thread.hpp>
#include <memory/Heap.hpp>
#include <memory/Memory.hpp>
#include <memory/Segment.hpp>
#include <proxys/Syscall.hpp>

Thread *Thread::running() { return scheduler_.current(); }

void Thread::dispatch(Thread *previous, Thread *next, Spin *lock) {
    next->state = State::RUNNING;
    lock->release();
    if (next != previous) {
        // if constexpr (Traits<System>::MULTITASK) {
        //     // next->task_->attach(previous->stack_);
        //     // next->task_->load();
        // }

        CPU::Atomic::wait(next->context_);
        CPU::Context::swtch(const_cast<CPU::Context **>(&previous->context_), CPU::Atomic::clear(next->context_));
    }
}

int Thread::idle(void *) {
    while (count_ > Traits<Machine>::CPUS) {
        if (!scheduler_.empty()) yield();
    }

    CPU::Interrupt::disable();
    if (CPU::core() == Traits<Machine>::BSP) Console::println("*** Shutdown! ***\n");
    for (;;);
    return 0;
}

// Thread::Thread(Task *t, Function f, Argument a, Criterion c) : task_(t), Thread(f, a, c) {}

// Thread::~Thread() {
//     lock_.lock();
//     switch (state) {
//         case (State::READY):
//             scheduler_.remove(&link);
//             count_ = count_ - 1;
//             break;
//         case (State::WAITING):
//             waiting->remove(&link);
//             count_ = count_ - 1;
//             break;
//         default:
//             break;
//     }
//
//     if (joining) {
//         joining->state = State::READY;
//         scheduler_.insert(&joining->link);
//     }
//
//     lock_.release();
//     Memory::kfree(stack);
// }

void Thread::join(Thread &thread) {
    auto previous = running();
    ERROR(&thread == previous, "[Thread::join] Join itself.");
    ERROR(thread.joining, "[Thread::join] Already joined.");

    lock_.lock();
    if (thread.state == State::FINISHED) {
        lock_.unlock();
        return;
    }

    previous->state = State::WAITING;
    thread.joining  = previous;

    dispatch(previous, scheduler_.pop(), &lock_);
    CPU::Interrupt::enable();
}

void Thread::exit() {
    lock_.lock();
    TraceIn();

    auto previous   = running();
    previous->state = State::FINISHED;

    if (previous->joining) {
        previous->joining->state = State::READY;
        scheduler_.insert(&previous->joining->link);
        previous->joining = 0;
    }

    count_ = count_ - 1;
    dispatch(previous, scheduler_.pop(), &lock_);
}

void Thread::init() {
    TraceIn();
    for (int i = 0; i < Traits<Machine>::CPUS; ++i) new (Heap::SYSTEM) Thread(idle, 0, Criterion::IDLE);
    TraceOut()
}

void Thread::run() {
    char buffer[sizeof(Thread)];
    Thread *previous = reinterpret_cast<Thread *>(buffer);
    lock_.acquire();
    TraceIn();
    Thread *next = scheduler_.pop();
    dispatch(previous, next, &lock_);
}

void Thread::reschedule() {
    if (scheduler_.empty()) return;

    auto previous   = running();
    previous->state = State::READY;

    lock_.acquire();
    scheduler_.insert(&previous->link);
    dispatch(previous, scheduler_.pop(), &lock_);
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

    lock_.acquire();
    auto next = scheduler_.pop();
    lock_.release();

    dispatch(previous, next, &lock);
}

void Thread::wakeup(Queue &waiting) {
    Element *awake = waiting.next();
    ERROR(!awake, "[Thread::wakeup] Empty queue.");
    awake->value->state   = State::READY;
    awake->value->waiting = nullptr;

    lock_.acquire();
    scheduler_.insert(awake);
    lock_.release();
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
