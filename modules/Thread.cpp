export module Thread;
import Machine;
import Memory;
import Logger;
import CPU;

template <typename T, typename P>
struct Queue {
    struct Node {
        Node *next;
        T *value;
    };

    void put(T *value) {
        P i         = value->priority;
        Node *item  = reinterpret_cast<Node *>(Memory::malloc(sizeof(Node), Memory::SYSTEM));
        item->value = value;
        item->next  = nullptr;

        Node *&head = priorities[i];

        if (!head) {
            head = item;
        } else {
            Node *current = head;
            while (current->next) {
                current = current->next;
            }
            current->next = item;
        }
    }

    T *get() {
        for (int i = P::MAX - 1; i >= 0; i--) {
            if (priorities[i]) {
                Node *item    = priorities[i];
                priorities[i] = item->next;
                T *value      = item->value;
                Memory::free(item, Memory::SYSTEM);
                return value;
            }
        }
        return nullptr;
    }

    Node *priorities[P::MAX];
};

export namespace Thread {
    enum Priority { IDLE, LOW, NORMAL, HIGH, MAX };
    enum State { RUNNING, READY, WAITING, FINISHED };
    struct Thread {
        uintptr_t stack;
        struct CPU::Context *context;
        struct Thread *joining;
        enum State state;
        enum Priority priority;
    };
    typedef Queue<Thread, Priority> Queue;

    void save(CPU::Context *);
    void exit();
    void init();
    void sleep(Queue *);
    void wakeup(Queue *);
    void yield();
    void timer_handler();
    void create(Thread *, int (*)(void *), void *, Priority);
    void join(Thread *);
};  // namespace Thread

extern int main(void *);
volatile Thread::Thread *_running;
int _count;
Thread::Queue _ready;
Thread::Thread _idle_thread;
Thread::Thread _user_thread;

void dispatch(Thread::Thread *current, Thread::Thread *next) {
    _running        = next;
    _running->state = Thread::RUNNING;
    CPU::Context::transfer(&current->context, next->context);
}

int idle(void *) {
    while (1) {
        CPU::Interrupt::disable();
        if (_count == 1) {
            Memory::kfree(reinterpret_cast<void *>(_running->stack));
            Memory::kfree(reinterpret_cast<void *>(_user_thread.stack));
            Logger::log("*** The last thread under control of QUARK has finished. ***\n");
            Logger::log("*** QUARK is shutting down! ***\n");
            while (1);
        } else {
            CPU::idle();
            Thread::yield();
        }
    }
    return 0;
}

void Thread::create(Thread *thread, int (*entry)(void *), void *arg, Priority priority) {
    thread->stack   = reinterpret_cast<uintptr_t>(Memory::kmalloc());
    thread->context = reinterpret_cast<CPU::Context *>(thread->stack + Machine::Memory::Page::SIZE);
    thread->context -= sizeof(CPU::Context);
    CPU::Context::create(thread->context, entry, exit, arg);
    thread->state    = READY;
    thread->priority = priority;
    _count++;
    _ready.put(thread);
}

void Thread::join(Thread *thread) {
    CPU::Interrupt::disable();
    if (thread->state != FINISHED) {
        Thread *previous = const_cast<Thread *>(_running);
        if (thread == previous) Logger::log("ERROR: Self join detected.");

        previous->state = WAITING;

        if (thread->joining) Logger::log("ERROR: Multiple joins detected.");
        thread->joining = previous;

        dispatch(previous, _ready.get());
    }

    Memory::kfree(reinterpret_cast<void *>(thread->stack));
    CPU::Interrupt::enable();
}

void Thread::exit() {
    CPU::Interrupt::disable();
    Thread *previous = const_cast<Thread *>(_running);
    if (previous->joining) _ready.put(previous->joining);
    previous->state = FINISHED;

    _count--;
    Thread *next = _ready.get();
    dispatch(previous, next);
}

void Thread::init() {
    create(&_user_thread, main, 0, NORMAL);
    create(&_idle_thread, idle, 0, IDLE);
    Thread *first   = _ready.get();
    _running        = first;
    _running->state = RUNNING;
    CPU::Context::jump(first->context);
}

void Thread::timer_handler() {
    Thread *previous = const_cast<Thread *>(_running);
    previous->state  = READY;
    _ready.put(previous);
    Thread *next    = _ready.get();
    _running        = next;
    _running->state = RUNNING;
    CPU::Context::jump(next->context);
}

void Thread::yield() {
    CPU::Interrupt::disable();
    Thread *previous = const_cast<Thread *>(_running);
    previous->state  = READY;
    _ready.put(previous);
    Thread *next = _ready.get();
    dispatch(previous, next);
}

void Thread::sleep(Queue *waiting) {
    Thread *previous = const_cast<Thread *>(_running);
    previous->state  = WAITING;
    waiting->put(previous);
    Thread *next = _ready.get();
    dispatch(previous, next);
}

void Thread::wakeup(Queue *waiting) {
    if (Thread *awake = waiting->get()) _ready.put(awake);
}

void Thread::save(CPU::Context *context) { _running->context = context; }
