#include <Alarm.hpp>
#include <IO/Debug.hpp>
#include <Thread.hpp>

extern int main(void *);
static Thread *_idle_thread;
static Thread *_user_thread;

void dispatch(Thread *next) {
    Thread *previous = const_cast<Thread *>(Thread::running());
    ERROR(next == nullptr, "[Thread::dispatch] Invalid thread.");
    ERROR(previous == next, "[Thread::dispatch] Same thread.");
    Thread::running(next);
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

Thread::Thread(Function f, Argument a, Rank r) {
    stack       = Memory::kmalloc();
    char *entry = reinterpret_cast<char *>(stack);
    entry += Traits<Memory>::Page::SIZE - sizeof(CPU::Context);
    context = new (entry) CPU::Context(f, exit, a);
    link    = new (Memory::SYSTEM) Element(this, r, nullptr);
    // rank    = r;
    state = State::READY;
    _count++;
    _scheduler.insert(this->link);
}

Thread::~Thread() {
    switch (state) {
        case (State::READY):
            _scheduler.remove(this->link);
            _count--;
            break;
        case (State::WAITING):
            waiting->remove(this->link);
            _count--;
            break;
        default:
            break;
    }
    Memory::kfree(stack);
}

void Thread::join(Thread *thread) {
    CPU::Interrupt::disable();
    if (thread->state != State::FINISHED) {
        Thread *previous = const_cast<Thread *>(running());

        ERROR(thread == previous, "[Thread::join] Join itself.");
        ERROR(thread->joining != nullptr, "[Thread::join] Already joined.");

        previous->state = State::WAITING;
        thread->joining = previous;
        dispatch(_scheduler.chose());
    }
}

void Thread::exit() {
    CPU::Interrupt::disable();
    Thread *previous = const_cast<Thread *>(running());
    if (previous->joining) _scheduler.insert(previous->joining->link);
    previous->state = State::FINISHED;
    _count--;
    Thread *next = _scheduler.chose();
    dispatch(next);
}

void Thread::init() {
    _idle_thread  = new (Memory::SYSTEM) Thread(idle, 0, IDLE);
    _user_thread  = new (Memory::SYSTEM) Thread(main, 0, NORMAL);
    Thread *first = _scheduler.chose();
    running(first);
    CPU::Context::jump(first->context);
}

void Thread::reschedule() {
    Thread *previous = const_cast<Thread *>(running());
    previous->state  = State::READY;
    _scheduler.insert(previous->link);
    Thread *next = _scheduler.chose();
    running(next);
}

void Thread::yield() {
    CPU::Interrupt::disable();
    Thread *previous = const_cast<Thread *>(running());
    previous->state  = State::READY;
    Thread *next     = _scheduler.chose();
    _scheduler.insert(previous->link);
    dispatch(next);
}

void Thread::sleep(Queue *waiting) {
    Thread *previous  = const_cast<Thread *>(running());
    previous->state   = State::WAITING;
    previous->waiting = waiting;
    waiting->insert(previous->link);
    Thread *next = _scheduler.chose();
    dispatch(next);
}

void Thread::wakeup(Queue *waiting) {
    Element *awake = waiting->next();
    ERROR(awake == nullptr, "[Thread::wakeup] Empty queue.");
    awake->value->state   = State::READY;
    awake->value->waiting = nullptr;
    _scheduler.insert(awake);
}

int entry(void *arg) {
    RT_Thread *current = const_cast<RT_Thread *>(reinterpret_cast<volatile RT_Thread *>(Thread::running()));
    auto now           = Alarm::utime();
    if (now < current->start) {
        Alarm::usleep(current->start - now);
    }

    while (1) {
        current->function(arg);

        now = Alarm::utime();
        if (now < current->start + current->deadline)
            Alarm::usleep(current->start + current->period - now);
        else
            ERROR(true, "Missed deadline by %d us\n", now - (current->start + current->deadline));

        current->start += current->period;
    }
    return 0;
}

RT_Thread::RT_Thread(Function f, Argument a, Microsecond p, Microsecond d, Microsecond c, Microsecond s)
    : Thread(entry, a, p), function(f), period(p), deadline(d), duration(c), start(s) {}
