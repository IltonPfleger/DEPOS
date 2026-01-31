#pragma once

#include <utils/Guard.hpp>

template <typename T>
concept Communicator = requires(T t, const unsigned char *data, unsigned int length) {
    { t.send(data, length) } -> Meta::SameAs<int>;
    { t.receive() } -> Meta::SameAs<Guard<typename T::ReceiveBuffer, &T::ReceiveBuffer::lock, &T::ReceiveBuffer::unlock>>;
};
