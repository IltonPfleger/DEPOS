#pragma once

#include <Meta.hpp>

template <typename Derived, bool Enabled>
class SystemResource {
   public:
    static void init() {
        static typename Meta::IF<Enabled, char[sizeof(Derived)], void*>::Result _MEMORY;

        if constexpr (Meta::ARRAY<Derived>) {
            
        }


        if constexpr (Enabled) {
            ::new (_MEMORY) Derived;
        }
    }

    static Derived* SYSTEM() { return _SYSTEM; }

   private:
    static inline Derived* _SYSTEM;
};
