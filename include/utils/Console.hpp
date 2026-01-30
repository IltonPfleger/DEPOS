#pragma once

#include <Meta.hpp>
#include <Traits.hpp>
#include <Variadic.hpp>

class Console {
    using IO = Meta::GetFromTypeList<Traits<UART>::Devices, 0>::Result;
    static void put(char);

  public:
    template <typename T> struct Hex {
      public:
        explicit Hex(T x) : m_x(x) {}
        operator T() const { return m_x; }

      private:
        T m_x;
    };

    static void panic();
    static void println(const char *, ...);
    static void print(char);
    static void print(const char *);
    static void print(unsigned long);
    template <typename T> static void print(Hex<T> x) {
        bool started = false;

        for (int i = (sizeof(x) * 8) - 4; i >= 0; i -= 4) {
            unsigned int current = (x >> i) & 0xF;
            if (current != 0 || started) {
                put(current < 10 ? current + '0' : current - 10 + 'a');
                started = true;
            }
        }
        if (!started) {
            put('0');
        }
    }

    template <Meta::Integer T> static void print(T t) {
        if constexpr (Meta::Signed<T>::Result) {
            if (t < 0) {
                put('-');
                t = -t;
            }
        }
        print(static_cast<unsigned long>(t));
    }

    template <Meta::Pointer T> static void print(T t) {
        Console::print("0x");
        Console::print(Hex(reinterpret_cast<unsigned long>(t)));
    }

    template <typename First, typename... Args> static void print(First &&first, Args &&...args) {
        Console::print(first);
        ((Console::print(", "), Console::print(args)), ...);
    }

  private:
    static inline int s_panic = -1;
};
