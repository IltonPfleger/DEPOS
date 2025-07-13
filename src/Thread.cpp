#include <Alarm.hpp>
#include <IO/Debug.hpp>
#include <Thread.hpp>
#include <Timer.hpp>

extern int main(void *);
static Thread *_idle_thread;
static Thread *_user_thread;

void dispatch(Thread *current, Thread *next) {
    ERROR(next == nullptr, "[Thread::dispatch] Invalid thread.");
    ERROR(current == next, "[Thread::dispatch] Same thread.");
    Thread::_running        = next;
    Thread::_running->state = Thread::RUNNING;
    CPU::Context::transfer(&current->context, next->context);
}

int idle(void *) {
    while (1) {
        if (Thread::_count == 1) {
            CPU::Interrupt::disable();
            delete _idle_thread;
            delete _user_thread;
            Logger::println("*** QUARK is shutting down! ***\n");
            while (1);
        } else {
            // CPU::idle();
            if (!Thread::_scheduler.empty()) Thread::yield();
        }
    }
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

RT_Thread::RT_Thread(int (*function)(void *), void *args, RT_Thread::Period period)
    : Thread(function, args, NORMAL), period(period) {}  //, last(Timer::time()) {}

void RT_Thread::wait_next() {
    volatile RT_Thread *running = reinterpret_cast<volatile RT_Thread *>(_running);
    Alarm::usleep(running->period);
    //
    //    running->last += running->period;
    //
    //    RT_Thread::Period now  = Timer::time(); //CONTANDO TICKS E DORMINDO EM MICROSEGUNDOS!!!
    //    RT_Thread::Period next = running->last;
    //
    //    if (now < next) {
    //        Alarm::udelay(next - now);
    //    } else {
    //        Logger::println("Missed deadline by %d us\n", now - next);
    //    }
}
