#pragma once

class Stack {
   public:
    template <auto Function, typename... Args>
    static void call(Args&&... args) {
        Function(args...);
    }
};
