#include <gp4/gp4.h>

static inline uint32_t parityFlag8(uint32_t value) {
    uint32_t p = value & 0xffu;
    p ^= p >> 4;
    p ^= p >> 2;
    p ^= p >> 1;
    return (p & 1u) == 0u ? 0x04u : 0u;
}

// Flags of a 16-bit logic result (xor / test): CF = OF = AF = 0.
static inline uint32_t logicFlags16(uint32_t result) {
    return parityFlag8(result)
        | ((result & 0xffffu) == 0u ? 0x40u : 0u)
        | ((result & 0x8000u) != 0u ? 0x80u : 0u);
}

// Flags of the 16-bit `left - right` (cmp).
static inline uint32_t subFlags16(uint32_t left, uint32_t right) {
    const uint32_t result = (left - right) & 0xffffu;
    return (left < right ? 0x01u : 0u)
        | parityFlag8(result)
        | (((left ^ right ^ result) & 0x10u) != 0u ? 0x10u : 0u)
        | (result == 0u ? 0x40u : 0u)
        | ((result & 0x8000u) != 0u ? 0x80u : 0u)
        | ((((left ^ right) & (left ^ result)) & 0x8000u) != 0u ? 0x800u : 0u);
}

// FUNCTION: GP4 0x0042e494
// Returns +1/-1 in eax (otherwise leaves eax alone) when |dx| < word 0x603b58, where dx is the
// difference of the word +0xe2 of the ebx and esi records (or the result of 0x0040dc8a when bit 1 of
// byte +0x9c is set on either record), and the words 0x604404/0x604406 have different signs: -1 when
// 0x604406 is negative. edx is restored; the flags are those of the last flag-setting instruction
// executed (cmp, xor, test, or the neg of eax).
void __cdecl FUN_0042e494(gp4::Regs* r) {
    uint8_t* first = reinterpret_cast<uint8_t*>(static_cast<uintptr_t>(r->esi));
    uint8_t* second = reinterpret_cast<uint8_t*>(static_cast<uintptr_t>(r->ebx));

    uint32_t dx;
    if ((GP4_FIELD(const uint8_t, first, 0x9c) & 2u) != 0u || (GP4_FIELD(const uint8_t, second, 0x9c) & 2u) != 0u) {
        gp4::Regs call = *r;
        call.st_in = 0;
        call.st_out = 0;
        gp4::call_regs(0x0040dc8a, call);
        dx = call.eax & 0xffffu;
    } else {
        dx = (GP4_FIELD(const uint16_t, second, 0xe2) - static_cast<uint32_t>(GP4_FIELD(const uint16_t, first, 0xe2))) & 0xffffu;
    }
    if ((dx & 0x8000u) != 0u) {
        dx = (0u - dx) & 0xffffu;
    }

    const uint32_t limit = GP4_GLOBAL(uint16_t, 0x00603b58);
    uint32_t flags = subFlags16(dx, limit);
    if (static_cast<int16_t>(dx) < static_cast<int16_t>(limit)) {
        const uint32_t signs = (GP4_GLOBAL(uint16_t, 0x00604404) ^ static_cast<uint32_t>(GP4_GLOBAL(uint16_t, 0x00604406))) & 0xffffu;
        flags = logicFlags16(signs);
        if ((signs & 0x8000u) != 0u) {
            const uint32_t direction = GP4_GLOBAL(uint16_t, 0x00604406);
            flags = logicFlags16(direction);
            if ((direction & 0x8000u) == 0u) {
                r->eax = 1u;
            } else {
                // neg eax of 1: CF = 1, AF = 1 (low nibble non-zero), result 0xffffffff
                r->eax = 0xffffffffu;
                flags = 0x01u | 0x04u | 0x10u | 0x80u;
            }
        }
    }
    r->eflags = (r->eflags & ~0x8d5u) | flags;
}
GP4_IMPL(0x0042e494, FUN_0042e494, "regs(eax:u32, ebx:ptr[0xea]:bytes, esi:ptr[0xea]:bytes) -> (eax, cf, pf, af, zf, sf, of); globals=fuzz:i32")
