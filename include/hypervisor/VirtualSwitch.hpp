#pragma once

#include <Spin.hpp>
#include <Traits.hpp>
#include <machine/Machine.hpp>

namespace DEPOS {

template <typename Device> class VirtualSwitch : public Device::Observer, public Device::Observed {
    static constexpr unsigned int FrameSize = 1518;
    static constexpr int InternalQueueSize  = 16;

    struct InternalBuffer {
        unsigned char data[FrameSize];
        unsigned int size;
        InternalBuffer()
            : size(0) {}
    };

  public:
    static auto instance() {
        static VirtualSwitch instance;
        return &instance;
    }

  public:
    VirtualSwitch()
        : device_(Device::instance()) {

        for (int i = 0; i < InternalQueueSize; i++) {
            free_.insert(&links_[i]);
        }

        device_->attach(this);
        thread_ = new Thread(entry, this);
    }

    NetworkBuffer *alloc(size_t length) { return device_->alloc(length); }

    int send(NetworkBuffer *buffer) {
        copy(buffer);
        device_->send(buffer, true);
        return 0;
    }

    void update(NetworkBuffer buffer) { copy(&buffer); }

  private:
    bool copy(NetworkBuffer *buffer) {
        bool enabled = CPU::Interrupt::disable();
        lock_.acquire();
        Link *link = free_.remove();
        lock_.release();
        if (enabled) CPU::Interrupt::enable();

        if (!link) return false;

        InternalBuffer &i = link->value();
        i.size            = buffer->length();
        memcpy(i.data, buffer->start(), buffer->length());

        enabled = CPU::Interrupt::disable();
        lock_.acquire();
        received_.insert(link);
        lock_.release();
        if (enabled) CPU::Interrupt::enable();

        return true;
    }

    static void *entry(void *p) { return reinterpret_cast<VirtualSwitch *>(p)->run(); }

    void *run() {
        while (true) {
            CPU::Interrupt::disable();
            lock_.acquire();
            Link *link = received_.remove();
            lock_.release();
            CPU::Interrupt::enable();

            if (link) {
                InternalBuffer &buf = link->value();

                NetworkBuffer temp_buffer(buf.data, 0, buf.size);
                this->notify(temp_buffer);

                CPU::Interrupt::disable();
                lock_.acquire();
                free_.insert(link);
                lock_.release();
                CPU::Interrupt::enable();
            }
        }
        return nullptr;
    }

  private:
    using Link = collections::Node<InternalBuffer>;
    using List = collections::FIFO<Link>;

    Device *device_;
    Thread *thread_;
    List free_{};
    List received_{};
    Link links_[InternalQueueSize];
    Spin lock_{};
};

} // namespace DEPOS
