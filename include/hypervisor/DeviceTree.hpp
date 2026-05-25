#ifndef __DEPOS_DEVICE_TREE__
#define __DEPOS_DEVICE_TREE__

#include <libraries/libc/string.h>

namespace DEPOS {

class DeviceTree {
    enum { FDT_BEGIN_NODE = 1, FDT_END_NODE = 2, FDT_PROP = 3, FDT_NOP = 4, FDT_END = 9 };

  public:
    bool valid() const;
    bool edit(const char *node_path, const char *property, const void *value, uint32_t size);

  private:
    size_t count(const char *path) const;
    void tokenize(const char *path, const char **components, size_t *lengths) const;
    bool match(const char *node_name, const char *component, size_t component_len) const;
    bool update(uint8_t *dest, const void *src, uint32_t size, uint32_t expected_len) const;

    uint32_t magic_;
    uint32_t total_size_;
    uint32_t structs_offset_;
    uint32_t strings_offset_;
    uint32_t mem_rsvmap_offset_;
    uint32_t version_;
    uint32_t last_comp_version_;
    uint32_t boot_cpuid_phys_;
    uint32_t strings_size_;
    uint32_t structs_size_;
};

inline bool DeviceTree::valid() const { return CPU::be32toh(magic_) == 0xD00DFEED; }

inline size_t DeviceTree::count(const char *path) const {
    size_t depth = 1;
    while (*path) {
        if (*path == '/') depth++;
        path++;
    }
    return depth;
}

inline void DeviceTree::tokenize(const char *path, const char **components, size_t *lengths) const {
    size_t index = 0;

    if (*path && *path == '/') path++;

    while (*path) {
        const char *start = path;

        while (*path && *path != '/')
            path++;

        components[index] = start;
        lengths[index]    = path - start;
        index++;
    }
}

inline bool DeviceTree::match(const char *node, const char *component, size_t length) const {
    return (strlen(node) == length && strcmp(node, component) == 0);
}

inline bool DeviceTree::update(uint8_t *destination, const void *source, uint32_t size, uint32_t expected) const {
    if (size == expected) {
        memcpy(destination, source, size);
        return true;
    }
    return false;
}

inline bool DeviceTree::edit(const char *node_path, const char *property, const void *value, uint32_t size) {
    uint32_t structs_offset = CPU::be32toh(structs_offset_);
    uint32_t structs_size   = CPU::be32toh(structs_size_);
    uint32_t strings_offset = CPU::be32toh(strings_offset_);

    uint8_t *base    = reinterpret_cast<uint8_t *>(this);
    uint8_t *structs = base + structs_offset;
    uint8_t *strings = base + strings_offset;
    uint8_t *current = structs;

    size_t target_depth = count(node_path);
    if (target_depth == 0) {
        return false;
    }

    const char **target_components = new const char *[target_depth];
    size_t *target_component_lens  = new size_t[target_depth];

    tokenize(node_path, target_components, target_component_lens);

    int32_t current_depth = -1;
    size_t matched_depth  = 0;
    bool success          = false;

    while (current < (structs + structs_size)) {
        uint32_t token = CPU::be32toh(*reinterpret_cast<uint32_t *>(current));
        current += 4;

        if (token == FDT_BEGIN_NODE) {
            current_depth++;
            const char *node_name = reinterpret_cast<const char *>(current);

            if (current_depth > 0 && matched_depth == static_cast<size_t>(current_depth - 1)) {
                if (match(node_name, target_components[matched_depth], target_component_lens[matched_depth])) {
                    matched_depth++;
                }
            }
            current += (strlen(node_name) + 1 + 3) & ~3;

        } else if (token == FDT_PROP) {
            uint32_t prop_len     = CPU::be32toh(*reinterpret_cast<uint32_t *>(current));
            uint32_t name_offset  = CPU::be32toh(*reinterpret_cast<uint32_t *>(current + 4));
            const char *prop_name = reinterpret_cast<const char *>(strings + name_offset);
            current += 8;

            if (matched_depth == target_depth && strcmp(prop_name, property) == 0) {
                if (update(current, value, size, prop_len)) {
                    success = true;
                    break;
                }
            }
            current += (prop_len + 3) & ~3;

        } else if (token == FDT_END_NODE) {
            if (matched_depth > 0 && matched_depth == static_cast<size_t>(current_depth)) {
                matched_depth--;
            }
            current_depth--;

        } else if (token == FDT_END) {
            break;
        } else if (token == FDT_NOP) {
            continue;
        }
    }

    delete[] target_components;
    delete[] target_component_lens;

    return success;
}

} // namespace DEPOS

#endif
