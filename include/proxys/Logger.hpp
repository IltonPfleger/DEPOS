#pragma once
#include <proxys/Proxy.hpp>

class Logger {
    template <typename, typename...>
    friend class Proxy;

    static void in() { Console::out << __PRETTY_FUNCTION__; };
    static void out() {};
};
