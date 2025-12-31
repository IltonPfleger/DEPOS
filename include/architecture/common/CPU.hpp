#pragma once

namespace ArchitectureCommon {

template <typename T>
concept CPUInterface = requires {
    { T::id() };
    { T::is_bsp() };
    { T::Barrier::wait() };
};

template <typename Atomic, typename Derived> class CPU {

  public:
    // static bool is_bsp() { return Derived::id() == Traits<CPU>::BSP; }

    class Barrier {

      public:
        static void wait(unsigned int cores = Traits<CPU>::ONLINE) {
            bool sense = gsense;
            unsigned int arrived = Atomic::finc(ready[sense]);

            if (arrived == cores - 1) {
                ready[sense] = 0;
                gsense = !sense;
            } else {
                while (gsense == sense)
                    ;
            }
        }

      private:
        static volatile inline bool gsense = true;
        static volatile inline unsigned int ready[2] = {0};
    };
};
} // namespace ArchitectureCommon
