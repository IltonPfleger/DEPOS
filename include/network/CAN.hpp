#ifndef __QUARK_NETWORK_CAN__
#define __QUARK_NETWORK_CAN__

#include <network/NetworkBuffer.hpp>
#include <types.hpp>

namespace QUARK {

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
        const uint32_t m_ide;
        const uint32_t m_rtr;
    };

    struct Frame {
        Header header;
        uint64_t data;
    };

  public:
    class Buffer : public NetworkBuffer {
      public:
        Buffer(uint32_t id = 0, size_t length = 8, bool ide = false, bool rtr = false)
            : NetworkBuffer(&m_frame.data, 0, length, nullptr),
              m_frame(Header(id, ide, rtr), 0) {}

        [[nodiscard]] const Header *header() const { return reinterpret_cast<const Header *>(&m_frame.header); }
        [[nodiscard]] uint32_t id() const { return header()->id(); }
        [[nodiscard]] bool isExtended() const { return header()->isExtended(); }
        [[nodiscard]] bool isRemote() const { return header()->isRemote(); }

      private:
        Frame m_frame;
    };
};

} // namespace QUARK

#endif
