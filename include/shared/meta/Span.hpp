// #ifndef __META_SPAN_HEADER__
// #define __META_SPAN_HEADER__
//
// #include <types.hpp>
//
// namespace DEPOS {
//
// namespace Meta {
//
// template <typename T> class Span {
//   public:
//     constexpr Span()
//         : m_data(nullptr),
//           m_size(0) {}
//
//     constexpr Span(T *data, size_t size)
//         : m_data(data),
//           m_size(size) {}
//
//     [[nodiscard]] constexpr const T *data() const { return m_data; }
//     [[nodiscard]] constexpr size_t size() const { return m_size; }
//     [[nodiscard]] constexpr bool empty() const { return m_size == 0; }
//     [[nodiscard]] constexpr const T &operator[](size_t i) const { return m_data[i]; }
//
//     [[nodiscard]] constexpr T *data() { return m_data; }
//     [[nodiscard]] constexpr T &operator[](size_t i) { return m_data[i]; }
//
//     [[nodiscard]] constexpr bool operator!=(const Span &other) const { return !(*this == other); }
//
//     [[nodiscard]] constexpr bool operator==(const Span &other) const {
//         if (m_size != other.m_size) return false;
//         for (size_t i = 0; i < m_size; ++i)
//             if (!(m_data[i] == other.m_data[i])) return false;
//         return true;
//     }
//
//   private:
//     T *m_data;
//     size_t m_size;
// };
//
// template <typename T> Span(T *, size_t) -> Span<T>;
//
// } // namespace Meta
//
// } // namespace DEPOS
//
// #endif
