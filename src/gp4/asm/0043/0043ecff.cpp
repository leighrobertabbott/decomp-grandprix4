#include <gp4/gp4.h>

// FUNCTION: GP4 0x0043ecff
void __cdecl FUN_0043ecff(gp4::Regs* r) {
    // Clamp check for a signed value in eax against a per-body limit:
    //   sign flag byte 0x627b61 = 0xff when eax < 0 (then eax is negated);
    //   limit = ((word[edi+0x60 or +0x62] + 0x50) * [0x627a9c]) >> 14;
    //   byte 0x627b62 = 0xff when |eax| < limit.
    // eax leaves negated (abs value), edx leaves the limit, flags are those of cmp.
    uint8_t* body = reinterpret_cast<uint8_t*>(static_cast<uintptr_t>(r->edi));
    uint32_t value = r->eax;
    uint32_t base;
    if (static_cast<int32_t>(value) < 0) {
        GP4_GLOBAL(uint8_t, 0x00627b61) = 0xff;
        value = 0u - value;
        base = GP4_FIELD(uint16_t, body, 0x60);
    } else {
        GP4_GLOBAL(uint8_t, 0x00627b61) = 0;
        base = GP4_FIELD(uint16_t, body, 0x62);
    }
    base += 0x50;
    const int64_t product = static_cast<int64_t>(static_cast<int32_t>(base))
                          * static_cast<int64_t>(GP4_GLOBAL(int32_t, 0x00627a9c));
    const uint32_t limit = static_cast<uint32_t>(static_cast<uint64_t>(product) >> 14);

    const uint32_t diff = value - limit;
    if (static_cast<int32_t>(value) < static_cast<int32_t>(limit)) {
        GP4_GLOBAL(uint8_t, 0x00627b62) = 0xff;
    } else {
        GP4_GLOBAL(uint8_t, 0x00627b62) = 0;
    }

    uint32_t parity = diff & 0xffu;
    parity ^= parity >> 4;
    parity ^= parity >> 2;
    parity ^= parity >> 1;
    const uint32_t flags = (value < limit ? 1u : 0u)
        | ((parity & 1u) == 0u ? 0x4u : 0u)
        | (((value ^ limit ^ diff) & 0x10u) != 0u ? 0x10u : 0u)
        | (diff == 0u ? 0x40u : 0u)
        | (diff & 0x80000000u ? 0x80u : 0u)
        | ((((value ^ limit) & (value ^ diff)) & 0x80000000u) ? 0x800u : 0u);

    r->eax = value;
    r->edx = limit;
    r->eflags = (r->eflags & ~0x8d5u) | flags;
}
GP4_IMPL(0x0043ecff, FUN_0043ecff, "regs(eax:i32, edi:ptr[0x70]:bytes) -> (eax, edx, cf, pf, af, zf, sf, of); globals=fuzz:i32")
