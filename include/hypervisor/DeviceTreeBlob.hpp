#pragma once

#include <architecture/CPU.hpp>
#include <shared/libraries/libc/string.h>
#include <types.hpp>

namespace DEPOS {

class DeviceTreeBlob {
    enum { FDT_BEGIN_NODE = 1, FDT_END_NODE = 2, FDT_PROP = 3, FDT_NOP = 4, FDT_END = 9 };

  public:
    bool valid() { return CPU::be32toh(m_magic) == 0xD00DFEED; }

    bool edit(const char *node, const char *property, const void *value, unsigned size) {
        unsigned structs_offset = CPU::be32toh(m_off_dt_struct);
        unsigned structs_size   = CPU::be32toh(m_size_dt_struct);
        unsigned strings_offset = CPU::be32toh(m_off_dt_strings);

        unsigned char *structs = reinterpret_cast<unsigned char *>(this) + structs_offset;
        unsigned char *strings = reinterpret_cast<unsigned char *>(this) + strings_offset;
        unsigned char *current = structs;
        bool is_target_node    = false;

        while (current < structs + structs_size) {
            unsigned token = CPU::be32toh(*(unsigned *)current);
            current += 4;

            if (token == FDT_BEGIN_NODE) {
                const char *name = (const char *)current;
                if (strcmp(name, node) == 0) {
                    is_target_node = true;
                } else {
                    is_target_node = false;
                }
                current += (strlen(name) + 1 + 3) & ~3;
            } else if (token == FDT_PROP) {
                unsigned prop_len     = CPU::be32toh(*(unsigned *)current);
                unsigned name_offset  = CPU::be32toh(*(unsigned *)(current + 4));
                const char *prop_name = (const char *)(strings + name_offset);
                current += 8;

                if (is_target_node && strcmp(prop_name, property) == 0) {
                    if (prop_len == size) {
                        memcpy(current, value, size);
                        current += (prop_len + 3) & ~3;
                        return true;
                    }
                }

                current += (prop_len + 3) & ~3;
            } else if (token == FDT_END_NODE) {
                is_target_node = false;
            } else if (token == FDT_END) {
                break;
            }
        }
        return false;
    }

  private:
    unsigned m_magic;
    unsigned m_totalsize;
    unsigned m_off_dt_struct;
    unsigned m_off_dt_strings;
    unsigned m_off_mem_rsvmap;
    unsigned m_version;
    unsigned m_last_comp_version;
    unsigned m_boot_cpuid_phys;
    unsigned m_size_dt_strings;
    unsigned m_size_dt_struct;
};

} // namespace DEPOS
