#include <gp4/gp4.h>

static inline uint32_t parityFlag8(uint32_t value) {
    uint32_t p = value & 0xffu;
    p ^= p >> 4;
    p ^= p >> 2;
    p ^= p >> 1;
    return (p & 1u) == 0u ? 0x04u : 0u;
}

// Flags of the 8-bit `cmp left, right` (operands are bytes).
static inline uint32_t cmpFlags8(uint32_t left, uint32_t right) {
    const uint32_t result = (left - right) & 0xffu;
    return (left < right ? 0x01u : 0u)
        | parityFlag8(result)
        | (((left ^ right ^ result) & 0x10u) != 0u ? 0x10u : 0u)
        | (result == 0u ? 0x40u : 0u)
        | ((result & 0x80u) != 0u ? 0x80u : 0u)
        | (((left ^ right) & (left ^ result) & 0x80u) != 0u ? 0x800u : 0u);
}

// Flags of an 8-bit logic result (CF = OF = AF = 0).
static inline uint32_t logicFlags8(uint32_t result) {
    return parityFlag8(result)
        | ((result & 0xffu) == 0u ? 0x40u : 0u)
        | (result & 0x80u);
}

// FUNCTION: GP4 0x0043c4f3
// esi = record with the 6-bit slot number in byte +0x7c. Stores the interpolated slot value of
// 0x0043ca66 (index (slot - 1), capped at 0x15) in [0x6227a0], refreshes the six curve results with
// 0x0043ca8b (eax = a level 0..5) and the minute count with 0x0043c4d9; a level below 2 is replaced by
// the cached level of 0x0043c475. When it differs from byte +0x7f it is stored in byte +0x160, and
// the first change also sets bit 2 of the flag byte +0x230. The flags are those of the last compare
// (or of the "or" that sets the flag bit).
void __cdecl FUN_0043c4f3(gp4::Regs* r) {
    uint8_t* body = reinterpret_cast<uint8_t*>(static_cast<uintptr_t>(r->esi));

    int32_t index = static_cast<int32_t>(GP4_FIELD(uint8_t, body, 0x7c) & 0x3fu) - 1;
    if (index > 0x15) {
        index = 0x15;
    }

    gp4::Regs s = *r;
    s.st_in = 0;
    s.st_out = 0;
    s.eax = static_cast<uint32_t>(index);
    gp4::call_regs(0x0043ca66, s);
    GP4_GLOBAL(uint32_t, 0x006227a0) = s.eax;
    gp4::call_regs(0x0043ca8b, s);
    gp4::call_regs(0x0043c4d9, s);
    if (static_cast<int8_t>(s.eax) < 2) {
        gp4::call_regs(0x0043c475, s);
    }

    const uint32_t level = s.eax & 0xffu;
    const uint32_t previous = GP4_FIELD(uint8_t, body, 0x7f);
    uint32_t flags = cmpFlags8(level, previous);
    if ((GP4_FIELD(uint8_t, body, 0x230) & 4u) == 0u) {
        if (level != previous) {
            const uint32_t marked = GP4_FIELD(uint8_t, body, 0x230) | 4u;
            GP4_FIELD(uint8_t, body, 0x230) = static_cast<uint8_t>(marked);
            flags = logicFlags8(marked);
            GP4_FIELD(uint8_t, body, 0x160) = static_cast<uint8_t>(level);
        }
    } else if (level != previous) {
        GP4_FIELD(uint8_t, body, 0x160) = static_cast<uint8_t>(level);
    }

    r->eax = s.eax;
    r->ebx = s.ebx;
    r->ecx = s.ecx;
    r->edx = s.edx;
    r->edi = s.edi;
    r->ebp = s.ebp;
    r->eflags = (r->eflags & ~0x8d5u) | flags;
}
GP4_IMPL(0x0043c4f3, FUN_0043c4f3, "regs(esi:ptr[0x238]:bytes) -> (eax, cf, pf, af, zf, sf, of); globals=fuzz:i32+layout=state_0043c1ba+state_0043c4d9")
