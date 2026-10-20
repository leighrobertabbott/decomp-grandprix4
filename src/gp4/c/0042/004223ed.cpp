#include <gp4/gp4.h>

static inline uint32_t parityFlag(uint32_t value) {
    uint32_t parity = value & 0xffu;
    parity ^= parity >> 4;
    parity ^= parity >> 2;
    parity ^= parity >> 1;
    return (parity & 1u) == 0u ? 4u : 0u;
}

// flags of cmp left, right
static inline uint32_t subFlags(uint32_t left, uint32_t right) {
    const uint32_t result = left - right;
    return parityFlag(result)
        | (((left ^ right ^ result) & 0x10u) != 0u ? 0x10u : 0u)
        | (result == 0u ? 0x40u : 0u)
        | ((result & 0x80000000u) != 0u ? 0x80u : 0u)
        | (((left ^ right) & (left ^ result) & 0x80000000u) != 0u ? 0x800u : 0u)
        | (left < right ? 1u : 0u);
}

// flags of test: CF = OF = AF = 0
static inline uint32_t testFlags(uint32_t result) {
    return parityFlag(result)
        | (result == 0u ? 0x40u : 0u)
        | ((result & 0x80000000u) != 0u ? 0x80u : 0u);
}

// FUNCTION: GP4 0x004223ed
void __cdecl FUN_004223ed(gp4::Regs* r) {
    uint8_t* block = (uint8_t*)GP4_GLOBAL(uint32_t, 0x006097b0);
    const uint32_t index = r->eax;
    const uint32_t kind = r->edx;
    const int32_t count = GP4_FIELD(int32_t, block, 0x18);
    uint32_t flags;
    if (count >= 0x48) {
        // list full: nothing is appended, eax keeps the count
        r->eax = (uint32_t)count;
        flags = subFlags((uint32_t)count, 0x48u);
    } else {
        GP4_FIELD(int32_t, block, 0x18) = count + 1;
        uint32_t* dst = (uint32_t*)(block + ((uint32_t)count * 0x14u + 0x38u));
        const uint32_t* src = (const uint32_t*)((index << 4) + GP4_GLOBAL(uint32_t, 0x0062a058));
        dst[0] = src[0] << 5;
        dst[1] = src[1] << 5;
        dst[2] = src[2] << 5;
        if ((kind & 0x14000u) != 0u) {
            dst[3] = 2u;
            flags = testFlags(kind & 0x14000u);
        } else {
            // the "test edx, 0x280" result is dead apart from the flags
            dst[3] = 1u;
            flags = testFlags(kind & 0x280u);
        }
        const uint32_t tail = GP4_GLOBAL(uint32_t, 0x006080cc);
        dst[4] = tail;
        r->eax = tail;
    }
    r->eflags = (r->eflags & ~0x8d5u) | flags;
}
GP4_IMPL(0x004223ed, FUN_004223ed, "regs(eax:u32[0..63], edx:u32) -> (eax, cf, pf, af, zf, sf, of); globals=layout=state_00422397+state_0042458b")
