#include <gp4/gp4.h>

// cmp left, right (32-bit) arithmetic flags: CF PF AF ZF SF OF.
static inline uint32_t subtractionFlags32(uint32_t left, uint32_t right) {
    const uint32_t result = left - right;
    uint32_t parity = result & 0xffu;
    parity ^= parity >> 4;
    parity ^= parity >> 2;
    parity ^= parity >> 1;
    return (left < right ? 1u : 0u)
        | ((parity & 1u) == 0u ? 4u : 0u)
        | (((left ^ right ^ result) & 0x10u) != 0u ? 0x10u : 0u)
        | (result == 0u ? 0x40u : 0u)
        | ((result & 0x80000000u) != 0u ? 0x80u : 0u)
        | (((left ^ right) & (left ^ result) & 0x80000000u) != 0u ? 0x800u : 0u);
}

// FUNCTION: GP4 0x004175ab
// Computes DAT_00602080 (a small positive count, stored as a word) as the maximum of
//   1, (DAT_0060384c + 0x2aa) / 0x555, (if record flag 8) (DAT_0060384c + 0x200) / 0x400,
//   a clamped 16-bit fixed-point chain on DAT_00637f68 / DAT_0060384c / DAT_00602c10 (+1, low byte),
//   DAT_00601d1c
// where every 16x16 imul keeps dx:ax shifted left (3 or 2 bits) and then only dx.
// DAT_00602c0c receives the intermediate magnitude word.
void __cdecl FUN_004175ab(gp4::Regs* r) {
    void* record = reinterpret_cast<void*>(static_cast<uintptr_t>(r->esi));

    int32_t ecx = 1;
    uint32_t ebp = 0x555u;
    const uint32_t first = (GP4_GLOBAL(uint32_t, 0x0060384c) + 0x2aau) / 0x555u;
    if (ecx < static_cast<int32_t>(first)) {
        ecx = static_cast<int32_t>(first);
    }
    if ((GP4_FIELD(const uint8_t, record, 0xe5) & 8u) != 0u) {
        ebp = 0x400u;
        const uint32_t second = (GP4_GLOBAL(uint32_t, 0x0060384c) + 0x200u) / 0x400u;
        if (ecx < static_cast<int32_t>(second)) {
            ecx = static_cast<int32_t>(second);
        }
    }

    // imul word [0x60384c] / three times shl ax,1 + rcl dx,1 / mov ax,dx
    const int16_t angle = GP4_GLOBAL(int16_t, 0x0060384c);
    const int16_t start = static_cast<int16_t>(GP4_GLOBAL(int32_t, 0x00637f68) >> 6);
    const int32_t product1 = static_cast<int32_t>(start) * static_cast<int32_t>(angle);
    const int16_t high1 = static_cast<int16_t>((static_cast<uint32_t>(product1) << 3) >> 16);

    // imul word [0x60384c] / shifted by 3 / dx
    const int32_t product2 = static_cast<int32_t>(high1) * static_cast<int32_t>(angle);
    uint16_t magnitude = static_cast<uint16_t>((static_cast<uint32_t>(product2) << 3) >> 16);
    if (static_cast<int16_t>(magnitude) < 0) {
        magnitude = static_cast<uint16_t>(0u - magnitude);   // neg dx
    }
    GP4_GLOBAL(uint16_t, 0x00602c0c) = magnitude;

    // imul word [0x602c10] / shifted by 2 / dx, clamped to 4, plus 1, low byte
    const int32_t product3 = static_cast<int32_t>(static_cast<int16_t>(magnitude))
        * static_cast<int32_t>(GP4_GLOBAL(int16_t, 0x00602c10));
    const uint16_t high3 = static_cast<uint16_t>((static_cast<uint32_t>(product3) << 2) >> 16);
    int16_t level = static_cast<int16_t>(high3);
    if (level > 4) {
        level = 4;
    }
    int32_t eax = static_cast<int32_t>(static_cast<uint16_t>(level + 1) & 0xffu);

    if (eax < ecx) {
        eax = ecx;
    }
    const uint32_t floor = GP4_GLOBAL(uint32_t, 0x00601d1c);
    const uint32_t flags = subtractionFlags32(static_cast<uint32_t>(eax), floor);
    if (eax < static_cast<int32_t>(floor)) {
        eax = static_cast<int32_t>(floor);
    }
    GP4_GLOBAL(uint16_t, 0x00602080) = static_cast<uint16_t>(eax);

    r->eax = static_cast<uint32_t>(eax);
    r->ecx = static_cast<uint32_t>(ecx);
    r->edx = high3;
    r->ebp = ebp;
    r->eflags = (r->eflags & ~0x8d5u) | flags;
}
GP4_IMPL(0x004175ab, FUN_004175ab, "regs(esi:ptr[0xf0]:bytes) -> (eax,ecx,edx,ebp,cf,pf,af,zf,sf,of); globals=fuzz:i32")
