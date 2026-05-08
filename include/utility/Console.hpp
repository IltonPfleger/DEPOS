#pragma once

#include <Meta.hpp>
#include <Traits.hpp>

namespace DEPOS {

class Console {
    using Device = Meta::GetFromTypeList<Traits<UART>::Devices, 0>::Result;

  private:
    static bool panicked();

  public:
    static void panic();

    template <typename T> struct Hex {
        constexpr Hex(T x)
            : m_value(x) {}

        constexpr operator T() { return m_value; }

      private:
        T m_value;
    };

    template <typename T> Hex(T) -> Hex<T>;

    static void print(char);

    static void print(const char *s) {
        while (s && *s)
            print(*s++);
    }

    static void print(Hex<uintmax_t> x) {
        if (x == 0) {
            print("0x0");
            return;
        }

        char buffer[64];
        int i              = 0;
        const char *digits = "0123456789abcdef";

        while (x > 0) {
            buffer[i++] = digits[x % 16];
            x           = x / 16;
        }

        print("0x");
        while (i > 0)
            print(buffer[--i]);
    }

    static void print(uintmax_t x) {
        if (x == 0) {
            print('0');
            return;
        }

        char buffer[64];
        int i         = 0;
        uintmax_t val = static_cast<uintmax_t>(x);

        while (val > 0) {
            buffer[i++] = (val % 10) + '0';
            val /= 10;
        }

        while (i > 0)
            print(buffer[--i]);
    }

    static void print(intmax_t x) {
        if (x < 0) {
            print('-');
            x = -x;
        }
        print(static_cast<uintmax_t>(x));
    }

    static void print(const void *p) {
        if (p == nullptr) {
            print("0x0");
        } else {
            print(Hex(reinterpret_cast<uintptr_t>(p)));
        }
    }

    template <Meta::Integer T> static void print(T x) {
        if constexpr (Meta::Signed<T>::Result) {
            print(static_cast<intmax_t>(x));
        } else {
            print(static_cast<uintmax_t>(x));
        }
    }
    template <Meta::Integer T> static void print(Hex<T> x) { print(Hex(static_cast<uintmax_t>(x))); }

    template <typename First, typename Second, typename... Others>
    static void print(First &&first, Second &&second, Others &&...others) {
        print(first);
        print(second, others...);
    }

    template <typename First, typename... Others> static void println(First &&first, Others &&...others) {
        print(first);
        if constexpr (sizeof...(others) > 0) {
            print(others...);
        }
        print('\n');
    }

  private:
    static volatile inline unsigned s_panic = 0;
    static inline unsigned s_column         = 0;
};

} // namespace DEPOS
