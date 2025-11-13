#pragma once

template <typename T>
concept Driver = requires(T a) {
    { a.write(...) } -> int;  // aceita qualquer número e tipo de argumentos
    { a.read(...) } -> int;
};
