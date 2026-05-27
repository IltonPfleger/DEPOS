#pragma once

namespace DEPOS::virtio {

struct LegacyHeader {
    unsigned int magic_value; // 0x000 (R) 'virt'
    unsigned int version;     // 0x004 (R) 1 for Legacy
    unsigned int device_id;   // 0x008 (R)
    unsigned int vendor_id;   // 0x00c (R) 0x554d4551

    unsigned int device_features;     // 0x010 (R)
    unsigned int device_features_sel; // 0x014 (W)

    unsigned int reserved_1[2]; // 0x018 - 0x020

    unsigned int driver_features;     // 0x020 (W)
    unsigned int driver_features_sel; // 0x024 (W)

    unsigned int guest_page_size; // 0x028 (W) Legacy only

    unsigned int reserved_2; // 0x02c

    unsigned int queue_sel;     // 0x030 (W)
    unsigned int queue_num_max; // 0x034 (R)
    unsigned int queue_num;     // 0x038 (W)
    unsigned int queue_align;   // 0x03c (W) Legacy only
    unsigned int queue_pfn;     // 0x040 (W) Legacy PFN

    unsigned int reserved_3[3]; // 0x044 - 0x050

    unsigned int queue_notify; // 0x050 (W)

    unsigned int reserved_4[3]; // 0x054 - 0x060

    unsigned int interrupt_status; // 0x060 (R)
    unsigned int interrupt_ack;    // 0x064 (W)

    unsigned int reserved_5[2]; // 0x068 - 0x070

    unsigned int status; // 0x070 (R/W)

    unsigned int reserved_6[35]; // 0x074 - 0x100
};

} // namespace DEPOS::virtio
