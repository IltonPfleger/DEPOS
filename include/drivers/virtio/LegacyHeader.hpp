#pragma once

namespace virtio {

class LegacyHeader {
  public:
    unsigned int m_magic;                   // 0x000 (R) 'virt'
    unsigned int m_version;                 // 0x004 (R) 1 for Legacy
    unsigned int m_id;                      // 0x008 (R)
    unsigned int m_vendor;                  // 0x00c (R) 0x554d4551
    unsigned int m_host_features;           // 0x010 (R)
    unsigned int m_host_features_selector;  // 0x014 (W)
    unsigned int m_reserved_1[2];           // 0x018 - 0x020
    unsigned int m_guest_features;          // 0x020 (W)
    unsigned int m_guest_features_selector; // 0x024 (W)
    unsigned int m_guest_page_size;         // 0x028 (W) Legacy Only
    unsigned int m_reserved_2;              // 0x02c
    unsigned int m_queue_selector;          // 0x030 (W)
    unsigned int m_queue_number_max;        // 0x034 (R)
    unsigned int m_queue_number;            // 0x038 (W) - Not Present in All Legacy Versions, Often Ignored or Used for Size
    unsigned int m_queue_align;             // 0x03c (W) Legacy Only
    unsigned int m_queue_page_frame_number; // 0x040 (W) Legacy PFN
    unsigned int m_reserved_3[3];           // 0x044 - 0x050
    unsigned int m_queue_notify;            // 0x050 (W)
    unsigned int m_reserved_4[3];           // 0x054 - 0x060
    unsigned int m_interrupt_status;        // 0x060 (R)
    unsigned int m_interrupt_ack;           // 0x064 (W)
    unsigned int m_reserved_5[2];           // 0x068 - 0x070
    unsigned int m_status;                  // 0x070 (R/W)
    unsigned int m_reserved_6[35];          // 0x074 - 0x100
};

} // namespace virtio
