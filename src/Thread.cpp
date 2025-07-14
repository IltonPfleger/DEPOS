#include <Alarm.hpp>
#include <IO/Debug.hpp>
#include <Thread.hpp>
#include <Timer.hpp>

extern int main(void *);
static Thread *_idle_thread;
static Thread *_user_thread;

void dispatch(Thread *next) {
    Thread *previous = const_cast<Thread *>(Thread::_running);
    ERROR(next == nullptr, "[Thread::dispatch] Invalid thread.");
    ERROR(previous == next, "[Thread::dispatch] Same thread.");
    Thread::_running        = next;
    Thread::_running->state = Thread::RUNNING;
    CPU::Context::transfer(&previous->context, next->context);
}

int idle(void *) {
    while (Thread::_count > 1) {
        CPU::idle();
        if (!Thread::_scheduler.empty()) Thread::yield();
    }

    CPU::Interrupt::disable();
    delete _idle_thread;
    delete _user_thread;
    Logger::println("*** QUARK is shutting down! ***\n");
    for (;;);
    return 0;
}

Thread::Thread(int (*function)(void *), void *args, Priority priority) {
    stack       = Memory::kmalloc();
    char *entry = reinterpret_cast<char *>(stack);
    entry += Traits<Memory>::Page::SIZE - sizeof(CPU::Context);
    context = new (entry) CPU::Context(function, exit, args);
    state   = READY;
    rank    = priority;
    _count++;
    _scheduler.insert(this);
}

Thread::~Thread() {
    switch (state) {
        case (READY):
            _scheduler.remove(this);
            _count--;
            break;
        case (WAITING):
            waiting->remove(this);
            _count--;
            break;
        default:
            break;
    }
    Memory::kfree(stack);
}

void Thread::join(Thread *thread) {
    CPU::Interrupt::disable();
    if (thread->state != FINISHED) {
        Thread *previous = const_cast<Thread *>(_running);

        ERROR(thread == previous, "[Thread::join] Join itself.");
        ERROR(thread->joining != nullptr, "[Thread::join] Already joined.");

        previous->state = WAITING;
        thread->joining = previous;
        dispatch(_scheduler.chose());
    }
}

void Thread::exit() {
    CPU::Interrupt::disable();
    Thread *previous = const_cast<Thread *>(_running);
    if (previous->joining) _scheduler.insert(previous->joining);
    previous->state = FINISHED;
    _count--;
    Thread *next = _scheduler.chose();
    dispatch(next);
}

void Thread::init() {
    _idle_thread    = new (Memory::SYSTEM) Thread(idle, 0, IDLE);
    _user_thread    = new (Memory::SYSTEM) Thread(main, 0, NORMAL);
    Thread *first   = _scheduler.chose();
    _running        = first;
    _running->state = RUNNING;
    CPU::Context::jump(first->context);
}

void Thread::reschedule() {
    Thread *previous = const_cast<Thread *>(_running);
    previous->state  = READY;
    _scheduler.insert(previous);
    Thread *next    = _scheduler.chose();
    _running        = next;
    _running->state = RUNNING;
}

void Thread::yield() {
    CPU::Interrupt::disable();
    Thread *previous = const_cast<Thread *>(_running);
    previous->state  = READY;
    Thread *next     = _scheduler.chose();
    _scheduler.insert(previous);
    dispatch(next);
}

void Thread::sleep(List *waiting) {
    Thread *previous  = const_cast<Thread *>(_running);
    previous->state   = WAITING;
    previous->waiting = waiting;
    waiting->insert(previous);
    Thread *next = _scheduler.chose();
    dispatch(next);
}

void Thread::wakeup(List *waiting) {
    Thread *awake = waiting->next();
    ERROR(awake == nullptr, "[Thread::wakeup] Empty queue.");
    awake->state   = READY;
    awake->waiting = nullptr;
    _scheduler.insert(awake);
}

RT_Thread::RT_Thread(int (*function)(void *), void *args, RT_Thread::Interval period)
    : Thread(function, args, NORMAL), period(period), next(Timer::utime() + period) {}

void RT_Thread::wait_next() {
    RT_Thread *running = const_cast<RT_Thread *>(reinterpret_cast<volatile RT_Thread *>(_running));

    RT_Thread::Time now = Timer::utime();

    if (now < running->next) {
        Alarm::usleep(running->next - now);
    } else {
        Logger::println("Missed deadline by %d us\n", now - running->next);
    }
    running->next += running->period;
}
