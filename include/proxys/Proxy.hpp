#pragma once

template <typename Class, typename... Proxys>
class Proxy {
   public:
    template <typename Function, typename... Args>
    static decltype(auto) call(Function function, Args&&... args) {
        (Proxys::in(), ...);
        function(static_cast<Args&&>(args)...);
        (Proxys::out(), ...);
    }
};
