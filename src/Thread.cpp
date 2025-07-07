#include <IO/Debug.hpp>
#include <Scheduler/Scheduler.hpp>
#include <Thread.hpp>

extern int main(void *);
volatile Thread *_running;
static int _count;
static Scheduler<Thread> _scheduler;
static Thread *_idle_thread;
static Thread *_user_thread;

void dispatch(Thread *current, Thread *next) {
    ERROR(next == nullptr, "[Thread::dispatch] Invalid thread.");
    ERROR(current == next, "[Thread::dispatch] Same thread.");
    _running        = next;
    _running->state = Thread::RUNNING;
    CPU::Context::transfer(&current->context, next->context);
}

int idle(void *) {
    while (1) {
        if (_count == 1) {
            CPU::Interrupt::disable();
            delete _idle_thread;
            delete _user_thread;
            Logger::log("*** QUARK is shutting down! ***\n");
            while (1);
        } else {
            CPU::Interrupt::enable();
            CPU::idle();
            if (!_scheduler.empty()) Thread::yield();
        }
    }
    return 0;
}

Thread::Thread(int (*entry)(void *), void *args, Priority p) {
    stack   = reinterpret_cast<uintptr_t>(Memory::kmalloc());
    context = reinterpret_cast<CPU::Context *>(stack + Traits<Memory>::Page::SIZE);
    context -= sizeof(CPU::Context);
    CPU::Context::create(context, entry, exit, args);
    state    = READY;
    priority = p;
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
    Memory::kfree(reinterpret_cast<void *>(stack));
}

Thread::Priority Thread::operator()() const { return priority; }

void Thread::join(Thread *thread) {
    CPU::Interrupt::disable();
    if (thread->state != FINISHED) {
        Thread *previous = const_cast<Thread *>(_running);

        ERROR(thread == previous, "[Thread::join] Join itself.");
        ERROR(thread->joining != nullptr, "[Thread::join] Already joined.");

        previous->state = WAITING;
        thread->joining = previous;
        dispatch(previous, _scheduler.chose());
    }
    CPU::Interrupt::enable();
}

void Thread::exit() {
    CPU::Interrupt::disable();
    Thread *previous = const_cast<Thread *>(_running);
    if (previous->joining) _scheduler.insert(previous->joining);
    previous->state = FINISHED;

    _count--;
    Thread *next = _scheduler.chose();
    dispatch(previous, next);
}

void Thread::init() {
    _idle_thread    = new (Memory::SYSTEM) Thread(idle, 0, IDLE);
    _user_thread    = new (Memory::SYSTEM) Thread(main, 0, NORMAL);
    Thread *first   = _scheduler.chose();
    _running        = first;
    _running->state = RUNNING;
    CPU::Context::jump(first->context);
}

void Thread::timer_handler() {
    Thread *previous = const_cast<Thread *>(_running);
    previous->state  = READY;
    _scheduler.insert(previous);
    Thread *next    = _scheduler.chose();
    _running        = next;
    _running->state = RUNNING;
    CPU::Context::jump(next->context);
}

void Thread::yield() {
    CPU::Interrupt::disable();
    Thread *previous = const_cast<Thread *>(_running);
    previous->state  = READY;
    Thread *next     = _scheduler.chose();
    _scheduler.insert(previous);
    dispatch(previous, next);
}

void Thread::sleep(List *waiting) {
    Thread *previous  = const_cast<Thread *>(_running);
    previous->state   = WAITING;
    previous->waiting = waiting;
    waiting->insert(previous);
    Thread *next = _scheduler.chose();
    dispatch(previous, next);
}

void Thread::wakeup(List *waiting) {
    Thread *awake = waiting->next();
    ERROR(awake == nullptr, "[Thread::wakeup] Empty queue.");
    awake->state   = READY;
    awake->waiting = nullptr;
    _scheduler.insert(awake);
}

void Thread::save(CPU::Context *context) { _running->context = context; }
