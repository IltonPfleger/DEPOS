#!/usr/bin/env python3
import socket
import sys
import struct

def main():
    if len(sys.argv) != 2:
        print(f"Uso: {sys.argv[0]} <interface>")
        sys.exit(1)

    iface = sys.argv[1]

    # Cria um raw socket de camada 2 (Ethernet)
    try:
        sock = socket.socket(socket.AF_PACKET, socket.SOCK_RAW)
        sock.bind((iface, 0))
    except PermissionError:
        print("Erro: é necessário rodar como root para usar raw sockets.")
        sys.exit(1)
    except Exception as e:
        print(f"Erro ao abrir socket: {e}")
        sys.exit(1)

    print(f"Enviando frames broadcast pela interface: {iface}")

    # MAC de destino: broadcast
    dst_mac = b'\xff\xff\xff\xff\xff\xff'
    
    # MAC de origem: você pode usar o real da interface
    # ou um MAC qualquer (spoofing) se for permitido.
    src_mac = b'\x12\x34\x56\x78\x9a\xbc'

    # EtherType arbitrário (0x0800 = IPv4, aqui uso 0x88B5 experimental)
    ethertype = struct.pack("!H", 0x88B5)

    # Payload (dados arbitrários)
    payload = b"Teste de broadcast raw socket"

    # Frame Ethernet completo
    frame = dst_mac + src_mac + ethertype + payload

    try:
        while True:
            sock.send(frame)
            print("Frame enviado.")
    except KeyboardInterrupt:
        print("\nEncerrado pelo usuário.")

if __name__ == "__main__":
    main()

