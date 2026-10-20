#include <gp4/gp4.h>

static inline uint32_t parityFlag(uint32_t value) {
    uint32_t parity = value & 0xffu;
    parity ^= parity >> 4;
    parity ^= parity >> 2;
    parity ^= parity >> 1;
    return (parity & 1u) == 0u ? 4u : 0u;
}

// Arithmetic flags (PF/AF/ZF/SF/OF, CF) left by the 16-bit `cmp word, 0x100`.
static inline uint32_t cmp16Flags(uint16_t left, uint16_t right) {
    const uint32_t result = (uint32_t)(uint16_t)(left - right);
    return (left < right ? 1u : 0u)
        | parityFlag(result)
        | (((left ^ right ^ result) & 0x10u) != 0u ? 0x10u : 0u)
        | (result == 0u ? 0x40u : 0u)
        | ((result & 0x8000u) != 0u ? 0x80u : 0u)
        | (((left ^ right) & (left ^ result) & 0x8000u) != 0u ? 0x800u : 0u);
}

// FUNCTION: GP4 0x004317f3
void __cdecl FUN_004317f3(gp4::Regs* r) {
    // Walks up to 20 of the 0x160-byte records after the one at [esi+0x1c], wrapping at the
    // ends of the two record ranges. CF set (and the walk stops) at the first record whose
    // signed word +0x14 is above 0x100; CF clear if none is. Other flags are those of that cmp.
    uint8_t* self = reinterpret_cast<uint8_t*>(static_cast<uintptr_t>(r->esi));
    uint32_t pos = GP4_FIELD(uint32_t, self, 0x1c);
    uint32_t count = 0x14;
    uint32_t flags = 0;
    bool found = false;
    for (;;) {
        pos += 0x160u;
        if (pos >= GP4_GLOBAL(uint32_t, 0x00866ecc)) {
            if (pos < GP4_GLOBAL(uint32_t, 0x00891500)) {
                pos -= GP4_GLOBAL(uint32_t, 0x00866ec4);
            } else if (pos >= GP4_GLOBAL(uint32_t, 0x00866ef8)) {
                pos = GP4_GLOBAL(uint32_t, 0x00866ef8) - 0x160u;
            }
        }
        const int16_t value = *reinterpret_cast<const int16_t*>(static_cast<uintptr_t>(pos + 0x14u));
        flags = cmp16Flags(static_cast<uint16_t>(value), 0x100u);
        if (value > 0x100) {
            found = true;
            break;
        }
        if (--count == 0) break;
    }
    r->eax = pos;
    r->ecx = count;
    r->eflags = (r->eflags & ~0x8d5u) | (flags & ~1u) | (found ? 1u : 0u);
}
GP4_IMPL(0x004317f3, FUN_004317f3, "regs(esi:u32[0x66fef0..0x66fef0]) -> (eax, ecx, cf, pf, af, zf, sf, of); globals=layout=ring_004317ae")
