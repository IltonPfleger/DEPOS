#include <Thread.hpp>
#include <abi/Handler.hpp>
#include <memory/Heap.hpp>

namespace QUARK::ABI {

void *Handler::dispatch(Function f, const Arguments a) {
    switch (f) {
        case Function::ABI_THREAD_CONSTRUCTOR: {
            auto ff = reinterpret_cast<Thread::Function>(a[0]);
            auto aa = reinterpret_cast<Thread::Argument>(a[1]);
            return new (Heap::SYSTEM) Thread(ff, aa);
        }
        case Function::ABI_THREAD_JOIN: {
            reinterpret_cast<Thread *>(a[0])->join();
            break;
        }
        case Function::ABI_THREAD_DESTRUCTOR: {
            delete reinterpret_cast<Thread *>(a[0]);
            break;
        };
        default: {
            assert(false, "Not Available!");
            break;
        };
    }

    return nullptr;
}

} // namespace QUARK::ABI
