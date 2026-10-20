#include <gp4/gp4.h>

static inline uint32_t parityFlag8(uint32_t value) {
    uint32_t parity = value & 0xffu;
    parity ^= parity >> 4;
    parity ^= parity >> 2;
    parity ^= parity >> 1;
    return (parity & 1u) == 0u ? 4u : 0u;
}

// Flags of `cmp a, b` (a - b, 32-bit).
static inline uint32_t cmpFlags(uint32_t a, uint32_t b) {
    const uint32_t result = a - b;
    return (a < b ? 1u : 0u)
        | parityFlag8(result)
        | (((a ^ b ^ result) & 0x10u) != 0u ? 0x10u : 0u)
        | (result == 0u ? 0x40u : 0u)
        | ((result & 0x80000000u) != 0u ? 0x80u : 0u)
        | ((((a ^ b) & (a ^ result)) & 0x80000000u) != 0u ? 0x800u : 0u);
}

// FUNCTION: GP4 0x00516874
// Record (esi) step/clamp: when the dword at +0x1c has reached the limit in 0x891500 the result
// is 2; otherwise the byte counter at +0x74 is incremented and wraps to 0 once it reaches 3.
// edx is saved and restored. Flags are those of the last cmp.
void __cdecl FUN_00516874(gp4::Regs* r) {
    const uint8_t* rec = (const uint8_t*)r->esi;
    const uint32_t counter = rec[0x74];
    const uint32_t value = *(const uint32_t*)(rec + 0x1c);
    const uint32_t limit = GP4_GLOBAL(uint32_t, 0x00891500);

    uint32_t result;
    uint32_t flags;
    if (value >= limit) {
        result = 2u;
        flags = cmpFlags(value, limit);
    } else {
        result = counter + 1u;
        flags = cmpFlags(result, 3u);
        if (result >= 3u) {
            result = 0u;
        }
    }
    r->eax = result;
    r->eflags = (r->eflags & ~0x8d5u) | flags;
}
GP4_IMPL(0x00516874, FUN_00516874, "regs(esi:ptr[0x7c]:bytes) -> (eax, cf, pf, af, zf, sf, of); globals=fuzz:i32")
