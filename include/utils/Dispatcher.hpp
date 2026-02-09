#pragma once

#pragma once

template <typename... Subscribers> struct Dispatcher {
    template <typename Event> static constexpr void broadcast(Event &e = {}) {
        (
            [&]() {
                if constexpr (requires { Subscribers::handler(e); }) {
                    Subscribers::handler(e);
                }
            }(),
            ...);
    }

    static constexpr void broadcast() {
        (
            []() {
                if constexpr (requires { Subscribers::handler(); }) {
                    Subscribers::handler();
                }
            }(),
            ...);
    }
};
