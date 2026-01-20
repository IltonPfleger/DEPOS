#pragma once

#include <architecture/rv/64/Context.hpp>

namespace rv {
#if __SIZEOF_POINTER__ == 8
template <typename Mode> using ContextBase = rv64::ContextBase<Mode>;
#else
#error "Invalid Context!"
#endif
} // namespace rv
