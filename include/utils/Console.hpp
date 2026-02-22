#pragma once

#include <Meta.hpp>
#include <Traits.hpp>
#include <Variadic.hpp>
#include <memory/Memory.hpp>

class Console {
    using IO = Meta::GetFromTypeList<Traits<UART>::Devices, 0>::Result;

  public:
    class Stream {
        friend Console;
        using Modifier = Stream &(*)(Stream &);

      public:
        Stream() : m_base(10) {}

        Stream &operator<<(Modifier modifier) { return modifier(*this); }

        Stream &operator<<(char c) { return put(c), *this; }

        Stream &operator<<(const char *s) {
            while (s && *s)
                *this << *s++;
            return *this;
        }

        Stream &operator<<(bool b) {
            *this << (b ? '1' : '0');
            return *this;
        }

        template <unsigned Length> Stream &operator<<(const char (&s)[Length]) { return *this << static_cast<const char *>(s); }

        template <Meta::Pointer T> Stream &operator<<(T p) { return *this << reinterpret_cast<void *>(p); }

        template <Meta::Integer T> Stream &operator<<(T x) {
            if constexpr (Meta::Signed<T>::Result) {
                if (m_base == 10 && x < 0) {
                    *this << '-';
                    x = static_cast<unsigned long>(-x);
                }
            }

            if (x == 0) {
                *this << '0';
                return *this;
            }

            char buffer[64];
            int i = 0;
            const char *digits = "0123456789abcdef";

            while (x > 0) {
                buffer[i++] = digits[x % m_base];
                x /= m_base;
            }

            if (m_base == 16) *this << "0x";

            while (i > 0) {
                *this << buffer[--i];
            }

            return *this;
        }

        Stream &operator<<(void *p) {
            unsigned int old = m_base;
            m_base = 16;
            *this << reinterpret_cast<uintptr_t>(p);
            m_base = old;
            return *this;
        }

      private:
        unsigned int m_base;
    };

  private:
    static void put(char);

  public:
    static void init() { new (&cout) Stream; }
    static void panic();
    static bool panicked();
    static Stream &endl(Console::Stream &s) { return (s << '\n' << dec); }
    static Stream &hex(Stream &s) { return s.m_base = 16, s; }
    static Stream &dec(Stream &s) { return s.m_base = 10, s; }
    static Stream &panic(Stream &s) { return Console::panic(), s; }

  public:
    static inline Stream cout;

  private:
    static volatile inline unsigned long s_panic = 0;
    static inline unsigned int s_column = 0;
};
