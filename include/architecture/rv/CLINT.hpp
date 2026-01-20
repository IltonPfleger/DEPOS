#include <architecture/rv/64/CLINT.hpp>

namespace rv {
#if __SIZEOF_POINTER__ == 8
using CLINT = rv64::CLINT;
#else
#error "Invalid CLINT!"
#endif
} // namespace rv
