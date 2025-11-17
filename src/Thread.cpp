#include <Alarm.hpp>
#include <IO/Debug.hpp>
#include <Thread.hpp>
#include <memory/Heap.hpp>
#include <memory/Memory.hpp>
#include <memory/Segment.hpp>
#include <proxys/Syscall.hpp>

Thread *Thread::running() { return scheduler_s.current(); }

void Thread::dispatch(Thread *previous, Thread *next, Spin *lock) {
    next->state = State::RUNNING;
    lock->release();
    if (next != previous) {
        // if constexpr (Traits<System>::MULTITASK) {
        //     // next->task_->attach(previous->stack_);
        //     // next->task_->load();
        // }

        CPU::Atomic::wait(next->context_);
        CPU::Context::swtch(const_cast<CPU::Context **>(&previous->context_),
                            CPU::Atomic::clear(next->context_));
    }
}

int Thread::idle(void *) {
    while (count_s > Traits<Machine>::CPUS) {
        if (!scheduler_s.empty())
            yield();
    }

    CPU::Interruptions::disable();
    if (CPU::id() == Traits<Machine>::BSP)
        Console::println("*** Shutdown! ***\n");
    for (;;)
        ;
    return 0;
}

// Thread::Thread(Task *t, Function f, Argument a, Criterion c) : task_(t),
// Thread(f, a, c) {}

// Thread::~Thread() {
//     lock_s.lock();
//     switch (state) {
//         case (State::READY):
//             scheduler_s.remove(&link);
//             count_s = count_s - 1;
//             break;
//         case (State::WAITING):
//             waiting->remove(&link);
//             count_s = count_s - 1;
//             break;
//         default:
//             break;
//     }
//
//     if (joining) {
//         joining->state = State::READY;
//         scheduler_s.insert(&joining->link);
//     }
//
//     lock_s.release();
//     Memory::kfree(stack);
// }

void Thread::join(Thread &thread) {
    auto previous = running();
    ERROR(&thread == previous, "[Thread::join] Join itself.");
    ERROR(thread.joining, "[Thread::join] Already joined.");

    lock_s.lock();
    if (thread.state == State::FINISHED) {
        lock_s.unlock();
        return;
    }

    previous->state = State::WAITING;
    thread.joining = previous;

    dispatch(previous, scheduler_s.pop(), &lock_s);
    CPU::Interruptions::enable();
}

void Thread::exit() {
    lock_s.lock();
    //TraceIn();

    auto previous = running();
    previous->state = State::FINISHED;

    if (previous->joining) {
        previous->joining->state = State::READY;
        scheduler_s.insert(&previous->joining->link);
        previous->joining = 0;
    }

    count_s = count_s - 1;
    //TraceOut();
    dispatch(previous, scheduler_s.pop(), &lock_s);
}

void Thread::init() {
    TraceIn();
    for (int i = 0; i < Traits<Machine>::CPUS; ++i)
        new (Heap::SYSTEM) Thread(idle, 0, Criterion::IDLE);
    TraceOut()
}

void Thread::run() {
    char buffer[sizeof(Thread)];
    Thread *previous = reinterpret_cast<Thread *>(buffer);
    lock_s.acquire();
    TraceIn();
    Thread *next = scheduler_s.pop();
    dispatch(previous, next, &lock_s);
}

void Thread::reschedule() {
    if (scheduler_s.empty())
        return;

    auto previous = running();
    previous->state = State::READY;

    lock_s.acquire();
    scheduler_s.insert(&previous->link);
    dispatch(previous, scheduler_s.pop(), &lock_s);
}

void Thread::yield() {
    CPU::Interruptions::disable();
    reschedule();
    CPU::Interruptions::enable();
}

void Thread::sleep(Queue &waiting, Spin &lock) {
    auto previous = running();
    previous->state = State::WAITING;
    previous->waiting = &waiting;
    waiting.insert(&previous->link);

    lock_s.acquire();
    auto next = scheduler_s.pop();
    lock_s.release();

    dispatch(previous, next, &lock);
}

void Thread::wakeup(Queue &waiting) {
    Element *awake = waiting.next();
    ERROR(!awake, "[Thread::wakeup] Empty queue.");
    awake->value->state = State::READY;
    awake->value->waiting = nullptr;

    lock_s.acquire();
    scheduler_s.insert(awake);
    lock_s.release();
}

// int entry(void *arg) {
//     RT_Thread *current = const_cast<RT_Thread *>(static_cast<volatile
//     RT_Thread *>(Thread::running())); auto now           = Alarm::utime(); if
//     (now < current->start) Alarm::usleep(current->start - now);
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
// RT_Thread::RT_Thread(Function f, Argument a, Microsecond d, Microsecond p,
// Microsecond c, Microsecond s)
//     : Thread(entry, a, Criterion(d, p, c)), function(f), deadline(d),
//     period(p), duration(c), start(s) {}
