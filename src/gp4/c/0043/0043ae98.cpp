#include <gp4/gp4.h>

static inline uint32_t parityFlag(uint32_t value) {
    uint32_t parity = value & 0xffu;
    parity ^= parity >> 4;
    parity ^= parity >> 2;
    parity ^= parity >> 1;
    return (parity & 1u) == 0u ? 4u : 0u;
}

// Arithmetic flags of "cmp left, right" (32-bit).
static inline uint32_t subFlags32(uint32_t left, uint32_t right) {
    const uint32_t result = left - right;
    return (left < right ? 1u : 0u)
        | parityFlag(result)
        | (((left ^ right ^ result) & 0x10u) != 0u ? 0x10u : 0u)
        | (result == 0u ? 0x40u : 0u)
        | ((result & 0x80000000u) != 0u ? 0x80u : 0u)
        | ((((left ^ right) & (left ^ result)) & 0x80000000u) != 0u ? 0x800u : 0u);
}

// 0x78-byte records at 0x0063bd7c (only the field this routine reads).
struct Unknown_0063bd7c {
    uint8_t Unknown00[0x64];
    uint32_t Unknown64;
    uint8_t Unknown68[0x78 - 0x68];
};
static_assert(sizeof(Unknown_0063bd7c) == 0x78, "record stride");

// FUNCTION: GP4 0x0043ae98
// pushal/popal wrapper (every GPR is preserved). If the byte flag [0x0063bcfc + ecx] is
// clear it returns CF = 0. Otherwise it measures the signed distance, via 0x004062b1,
// between eax and the car record 0x008943a0 + rec[ecx].Unknown64 (rec = 0x0063bd7c +
// ecx * 0x78) and returns CF = 1 when |distance| < edx (signed), else CF = 0. The other
// flags are those of the last cmp (cmp byte [..], 0 for the clear case; cmp |distance|, edx).
void __cdecl FUN_0043ae98(gp4::Regs* r) {
    const uint32_t index = r->ecx;
    uint32_t flags;
    if (GP4_ARRAY(uint8_t, 0x0063bcfc)[index] == 0u) {
        flags = 0x44u;   // cmp byte, 0 with a zero byte: ZF | PF, CF cleared by clc
    } else {
        const Unknown_0063bd7c* rec = GP4_ARRAY(Unknown_0063bd7c, 0x0063bd7c) + index;
        gp4::Regs callee = *r;
        callee.esi = (uint32_t)(uintptr_t)rec;
        callee.edx = rec->Unknown64 + 0x008943a0u;
        callee.st_in = 0;
        callee.st_out = 0;
        gp4::call_regs(0x004062b1, callee);

        int32_t distance = (int32_t)callee.eax;
        if (distance < 0) {
            distance = (int32_t)(0u - (uint32_t)distance);
        }
        const int32_t limit = (int32_t)r->edx;
        flags = subFlags32((uint32_t)distance, (uint32_t)limit) & ~1u;
        if (distance < limit) {
            flags |= 1u;   // stc
        }
    }
    r->eflags = (r->eflags & ~0x8d5u) | flags;
}
GP4_IMPL(0x0043ae98, FUN_0043ae98, "regs(eax:u32, ecx:u32[0..63], edx:u32) -> (cf, pf, af, zf, sf, of); globals=layout=bytes_0063bcfc")
