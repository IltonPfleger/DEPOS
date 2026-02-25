#pragma once

#include <utils/Debug.hpp>

namespace DEPOS {

template <unsigned int First, unsigned int Last, typename Tag> class DispatchTable {
  public:
    using Handler = void (*)(unsigned int);

    static void dispatch(unsigned int id) {
        ERROR(id < First || id > Last, "ID: ", id);
        ERROR(s_entries[id - First] == 0, "ID: ", id);
        s_entries[id - First](id);
    }

    static void bind(unsigned int id, Handler handler) {
        ERROR(id < First || id > Last, "ID: ", id);
        s_entries[id - First] = handler;
    }

  private:
    static inline Handler s_entries[Last - First + 1];
};

} // namespace DEPOS
