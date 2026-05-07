#ifndef __DEPOS_UTILITY_SPAN__
#define __DEPOS_UTILITY_SPAN__

namespace DEPOS {

template <typename T> class Span {

    Span(T *data, size_t length)
        : _data(data),
          _length(length) {}

    [[nodiscard]] T &operator[](this auto &&self, size_t i) { return self._data[i]; }

  private:
    T *_data;
    size_t _length;
};

} // namespace DEPOS
#endif
