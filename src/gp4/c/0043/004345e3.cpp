#include <gp4/gp4.h>

static inline uint32_t parityFlag(uint32_t value) {
    value &= 0xffu;
    value ^= value >> 4;
    value ^= value >> 2;
    value ^= value >> 1;
    return ((value & 1u) == 0u) ? 4u : 0u;
}

// Flags left by a byte-wide TEST: CF = OF = AF = 0, ZF/SF/PF from the result.
static inline uint32_t logicFlags8(uint32_t result) {
    return parityFlag(result)
        | ((result & 0xffu) == 0u ? 0x40u : 0u)
        | ((result & 0x80u) != 0u ? 0x80u : 0u);
}

// Flags of the 32-bit `cmp left, right`.
static inline uint32_t subtractionFlags32(uint32_t left, uint32_t right) {
    const uint32_t result = left - right;
    return (left < right ? 1u : 0u)
        | parityFlag(result)
        | (((left ^ right ^ result) & 0x10u) != 0u ? 0x10u : 0u)
        | (result == 0u ? 0x40u : 0u)
        | ((result & 0x80000000u) != 0u ? 0x80u : 0u)
        | (((left ^ right) & (left ^ result) & 0x80000000u) != 0u ? 0x800u : 0u);
}

// FUNCTION: GP4 0x004345e3
// When bit 1 of byte 0x852d98 is set, esi is the car record in dword 0x66d820 and bit 1 of its byte
// +0x71 is clear, byte 0x6f2f64 = 0xff. The flags are those of the last test/cmp executed.
void __cdecl FUN_004345e3(gp4::Regs* r) {
    uint8_t* body = (uint8_t*)r->esi;
    uint32_t flags = logicFlags8(GP4_GLOBAL(uint8_t, 0x00852d98) & 2u);   // test byte [0x852d98], 2

    if ((GP4_GLOBAL(uint8_t, 0x00852d98) & 2u) != 0u) {
        const uint32_t other = GP4_GLOBAL(uint32_t, 0x0066d820);
        flags = subtractionFlags32(r->esi, other);                       // cmp esi, [0x66d820]
        if (r->esi == other) {
            const uint32_t masked = GP4_FIELD(uint8_t, body, 0x71) & 2u;
            flags = logicFlags8(masked);                                 // test byte [esi+0x71], 2
            if (masked == 0u) {
                GP4_GLOBAL(uint8_t, 0x006f2f64) = 0xff;
            }
        }
    }

    r->eflags = (r->eflags & ~0x8d5u) | flags;
}
GP4_IMPL(0x004345e3, FUN_004345e3, "regs(esi:rec[0x66fef0,0x41c,0..21]) -> (cf, pf, af, zf, sf, of); globals=layout=state_00434602+state_0040edc3")
