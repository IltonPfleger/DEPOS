#pragma once

#include <machine/aes.h>
#include <machine/nic.h>
#include <utility/observer.h>

class UDP_Socket : public Observed {};

// Classe UDPNIC mínima compilável
class UDPNIC : public NIC<Only_Data_UDP_Wrapper>, public Observer {
    static const UInt32 KEY_SIZE = Traits<TSTP>::KEY_SIZE;
    static const UInt32 MTU = NIC<Only_Data_UDP_Wrapper>::MTU; // 64KB

    typedef AES<KEY_SIZE> _AES;
    static _AES _aes;

  public:
    UDPNIC() = default;
    ~UDPNIC() override = default;

    // Implementações mínimas das funções virtuais puras da NIC
    int send(const Address &dst, const Protocol &prot, const void *data, UInt32 size) override {
        // TODO: implementar envio real
        return 0;
    }

    int receive(Address *src, Protocol *prot, void *data, UInt32 size) override {
        // TODO: implementar recebimento real
        return 0;
    }

    Buffer *alloc(const Address &dst, const Protocol &prot, UInt32 once, UInt32 always, UInt32 payload) override {
        // TODO: criar buffer real
        return nullptr;
    }

    int send(Buffer *buf) override {
        // TODO: enviar buffer real
        return 0;
    }

    void free(Buffer *buf) override {
        // TODO: liberar buffer real
    }

    const Address &address() override {
        static Address dummy{};
        return dummy;
    }

    void address(const Address &addr) override {
        // TODO: armazenar endereço real
    }

    bool reconfigure(const Configuration *c = nullptr) override {
        return true; // TODO: aplicar configuração real
    }

    const Configuration &configuration() override {
        static Configuration dummy{};
        return dummy;
    }

    const Statistics &statistics() override {
        static Statistics dummy{};
        return dummy;
    }

    void update(typename UDP_Socket::Observed *obs) override {
        // TODO: processar notificações do socket
    }

  private:
    static UDP_Socket *soc;
    static unsigned char GRP_KEY[16];
};
