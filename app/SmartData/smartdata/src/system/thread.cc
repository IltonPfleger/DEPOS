#include <main_traits.h>
#include<system/thread.h>

std::unordered_map<pthread_t, Thread*> * Thread::_pthread_thread_umap;

void Thread::init() {
    _pthread_thread_umap = new std::unordered_map<pthread_t, Thread*>();
}

void Thread::finish() {
    delete _pthread_thread_umap;
}
