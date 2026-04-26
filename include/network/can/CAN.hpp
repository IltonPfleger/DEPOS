#ifndef __NETWORK_CAN_HEADER__
#define __NETWORK_CAN_HEADER__

#include <network/NetworkBuffer.hpp>
#include <types.hpp>

namespace DEPOS {

class CAN {
    class Header {
      public:
        Header(uint32_t id, bool ide, bool rtr)
            : m_id(id),
              m_ide(ide),
              m_rtr(rtr) {}

        [[nodiscard]] uint32_t id() const { return m_id; }
        [[nodiscard]] bool ide() const { return m_ide; }
        [[nodiscard]] bool rtr() const { return m_rtr; }

        [[nodiscard]] bool isExtended() const { return ide(); }
        [[nodiscard]] bool isRemote() const { return rtr(); }

      private:
        const uint32_t m_id;
        const bool m_ide;
        const bool m_rtr;
    };

  public:
    class Buffer : public NetworkBuffer {
      public:
        Buffer(uint32_t id, bool ide = false, bool rtr = false)
            : NetworkBuffer(m_data, Size) {
            new (data()) Header(id, ide, rtr);
            advance(sizeof(Header));
        }

        [[nodiscard]] const Header *header() const { return reinterpret_cast<const Header *>(m_data); }
        [[nodiscard]] uint32_t id() const { return header()->id(); }
        [[nodiscard]] bool isExtended() const { return header()->isExtended(); }
        [[nodiscard]] bool isRemote() const { return header()->isRemote(); }

      private:
        static constexpr size_t Size = sizeof(Header) + 8;
        unsigned char m_data[Size];
    };
};

} // namespace DEPOS

#endif
