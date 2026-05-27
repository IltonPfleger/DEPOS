#pragma once

namespace DEPOS {

namespace virtio {

class LegacyHeader {
  public:
    unsigned int magic;                     // 0x000 (R) 'virt'
    unsigned int version;                   // 0x004 (R) 1 for Legacy
    unsigned int id;                        // 0x008 (R)
    unsigned int vendor;                    // 0x00c (R) 0x554d4551
    unsigned int host_features;             // 0x010 (R)
    unsigned int host_features_selector;    // 0x014 (W)
    unsigned int reserved_1[2];             // 0x018 - 0x020
    unsigned int guest_features;            // 0x020 (W)
    unsigned int guest_features_selector;   // 0x024 (W)
    unsigned int guest_page_size;           // 0x028 (W) Legacy Only
    unsigned int reserved_2;                // 0x02c
    unsigned int queue_selector;            // 0x030 (W)
    unsigned int max_number_of_descriptors; // 0x034 (R)
    unsigned int queue_length;              // 0x038 (W)
    unsigned int queue_align;               // 0x03c (W) Legacy Only
    unsigned int queue_page_frame_number;   // 0x040 (W) Legacy PFN
    unsigned int reserved_3[3];             // 0x044 - 0x050
    unsigned int queue_notify;              // 0x050 (W)
    unsigned int reserved_4[3];             // 0x054 - 0x060
    unsigned int interrupt_status;          // 0x060 (R)
    unsigned int interrupt_ack;             // 0x064 (W)
    unsigned int reserved_5[2];             // 0x068 - 0x070
    unsigned int status;                    // 0x070 (R/W)
    unsigned int reserved_6[35];            // 0x074 - 0x100
};

} // namespace virtio

} // namespace DEPOS
