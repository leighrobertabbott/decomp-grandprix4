#include <gp4/gp4.h>

static inline uint32_t parityFlag(uint32_t value) {
    uint32_t parity = value & 0xffu;
    parity ^= parity >> 4;
    parity ^= parity >> 2;
    parity ^= parity >> 1;
    return (parity & 1u) == 0u ? 4u : 0u;
}

static inline uint32_t resultFlags(uint32_t res) {
    return parityFlag(res) | (res == 0u ? 0x40u : 0u) | ((res & 0x80000000u) != 0u ? 0x80u : 0u);
}

// Flags of the 32-bit `a + b`.
static inline uint32_t addFlags32(uint32_t a, uint32_t b) {
    const uint32_t res = a + b;
    uint32_t f = resultFlags(res);
    if (res < a) f |= 0x01u;
    if (((a ^ b ^ res) & 0x10u) != 0u) f |= 0x10u;
    if ((~(a ^ b) & (a ^ res) & 0x80000000u) != 0u) f |= 0x800u;
    return f;
}

// Flags of the 32-bit `a - b` (cmp / sub).
static inline uint32_t subFlags32(uint32_t a, uint32_t b) {
    const uint32_t res = a - b;
    uint32_t f = resultFlags(res);
    if (a < b) f |= 0x01u;
    if (((a ^ b ^ res) & 0x10u) != 0u) f |= 0x10u;
    if (((a ^ b) & (a ^ res) & 0x80000000u) != 0u) f |= 0x800u;
    return f;
}

// Advances the track-segment pointer in eax by the number of whole 0x160-byte segments the car will cover:
// n = (max(0, result of 0x00426261) + 0x2000) >> 14. Before the pit start [0x891500] the pointer wraps by
// [0x866ec4] when it reaches the lap end [0x866ecc]; from the pit start on it is limited to the segment
// before the pit end [0x866ef8]. ebx, ecx and edi are saved; edx and ebp are whatever 0x00426261 leaves.
// FUNCTION: GP4 0x00426320
void __cdecl FUN_00426320(gp4::Regs* r) {
    gp4::Regs call = *r;
    call.st_in = 0;
    call.st_out = 0;
    call.ebx = r->eax;
    gp4::call_regs(0x00426261, call);

    uint32_t segment = r->eax;
    int32_t distance = static_cast<int32_t>(call.eax);
    if (distance < 0) distance = 0;
    const uint32_t count = (static_cast<uint32_t>(distance) + 0x2000u) >> 14;

    uint32_t flags;
    if (count == 0u) {
        // or eax, eax with eax = 0
        flags = 0x44u;
    } else {
        const uint32_t step = count * 0x160u;
        const uint32_t pitStart = GP4_GLOBAL(uint32_t, 0x00891500);
        flags = subFlags32(segment, pitStart);
        if (segment >= pitStart) {
            const uint32_t advanced = segment + step;
            const uint32_t pitEnd = GP4_GLOBAL(uint32_t, 0x00866ef8);
            flags = subFlags32(advanced, pitEnd);
            segment = advanced;
            if (advanced >= pitEnd) {
                segment = pitEnd - 0x160u;
                flags = subFlags32(pitEnd, 0x160u);
            }
        } else {
            const uint32_t advanced = segment + step;
            const uint32_t lapEnd = GP4_GLOBAL(uint32_t, 0x00866ecc);
            flags = subFlags32(advanced, lapEnd);
            segment = advanced;
            if (advanced >= lapEnd) {
                const uint32_t wrap = GP4_GLOBAL(uint32_t, 0x00866ec4);
                segment = advanced - wrap;
                flags = subFlags32(advanced, wrap);
            }
        }
    }

    r->eax = segment;
    r->edx = call.edx;
    r->ebp = call.ebp;
    r->eflags = (r->eflags & ~0x8d5u) | (flags & 0x8d5u);
}
GP4_IMPL(0x00426320, FUN_00426320, "regs(eax:rec[0x8943a0,0x160,0..25], edx:u32, esi:ptr[0x273]:layout=car_00426320) -> (eax, edx, cf, pf, af, zf, sf, of); globals=fuzz:i32+layout=ring_008943a0+ring_0040c0ae+look_00426320")
