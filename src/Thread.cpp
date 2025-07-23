#include <Alarm.hpp>
#include <IO/Debug.hpp>
#include <Thread.hpp>

extern int main(void *);
static Thread *_user_thread;

__attribute__((naked)) void dispatch(Thread *previous, Thread *next) {
    CPU::Context::push();
    previous->context = CPU::Context::get();

    ERROR(next == nullptr, "[Thread::dispatch] Invalid thread.");
    ERROR(previous == next, "[Thread::dispatch] Same thread.");
    Thread::running(next);

    Thread::lock.unlock();
    CPU::Context::jump(next->context);
}

int Thread::idle(void *) {
    while (_count > Machine::CPUS) {
        if (!_scheduler.empty()) yield();
    }

    CPU::Interrupt::disable();
    delete running();
    if (CPU::core() == 0) {
        delete _user_thread;
        lock.lock();
        Logger::println("*** QUARK is shutting down! ***\n");
        lock.unlock();
    }
    for (;;);
    return 0;
}

Thread::Thread(Function f, Argument a, Criterion c) : state(State::READY), criterion(c) {
    stack       = Memory::kmalloc();
    link        = new (Memory::SYSTEM) Element(this, c.priority());
    char *entry = reinterpret_cast<char *>(stack) + Traits<Memory>::Page::SIZE - sizeof(CPU::Context);
    context     = new (entry) CPU::Context(f, exit, a);

    lock.lock();
    _scheduler.insert(this->link);
    _count = _count + 1;
    lock.unlock();
}

Thread::~Thread() {
    lock.lock();
    switch (state) {
        case (State::READY):
            _scheduler.remove(this->link);
            _count = _count - 1;
            break;
        case (State::WAITING):
            waiting->remove(this->link);
            _count = _count - 1;
            break;
        default:
            break;
    }
    lock.unlock();
    Memory::kfree(stack);
}

void Thread::join(Thread *thread) {
    if (thread->state == State::FINISHED) return;

    Thread *previous = const_cast<Thread *>(running());
    ERROR(thread == previous, "[Thread::join] Join itself.");
    ERROR(thread->joining != nullptr, "[Thread::join] Already joined.");
    previous->state = State::WAITING;
    thread->joining = previous;

    lock.lock();
    Thread *next = _scheduler.chose();
    dispatch(previous, next);
}

void Thread::exit() {
    Thread *previous = const_cast<Thread *>(running());
    previous->state  = State::FINISHED;
    lock.lock();
    if (previous->joining) {
        previous->joining->state = State::READY;
        _scheduler.insert(previous->joining->link);
        previous->joining = 0;
    }
    _count       = _count - 1;
    Thread *next = _scheduler.chose();
    dispatch(previous, next);
}

void Thread::init() { _user_thread = new (Memory::SYSTEM) Thread(main, 0, NORMAL); }

void Thread::core() {
    while (_count != 1);
    new (Memory::SYSTEM) Thread(idle, 0, IDLE);
    while (_count != Machine::CPUS + 1);
    lock.lock();
    Thread *first = _scheduler.chose();
    running(first);
    lock.unlock();
    CPU::Context::jump(first->context);
}

void Thread::yield() {
    Thread *previous = const_cast<Thread *>(running());
    previous->state  = State::READY;
    lock.lock();
    Thread *next = _scheduler.chose();
    if (!next) {
        lock.unlock();
        return;
    }
    _scheduler.insert(previous->link);
    dispatch(previous, next);
}

// void Thread::reschedule() {
//     Thread *previous = const_cast<Thread *>(running());
//     previous->state  = State::READY;
//     _scheduler.insert(previous->link);
//     Thread *next = _scheduler.chose();
//     running(next);
// }

// void Thread::sleep(Queue *waiting) {
//     Thread *previous  = const_cast<Thread *>(running());
//     previous->state   = State::WAITING;
//     previous->waiting = waiting;
//     waiting->insert(previous->link);
//     Thread *next = _scheduler.chose();
//     dispatch(next);
// }

// void Thread::wakeup(Queue *waiting) {
//     Element *awake = waiting->next();
//     ERROR(awake == nullptr, "[Thread::wakeup] Empty queue.");
//     awake->value->state   = State::READY;
//     awake->value->waiting = nullptr;
//     _scheduler.insert(awake);
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
//		int miss = (now - current->start) - current->deadline;
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
