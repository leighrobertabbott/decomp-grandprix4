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

// FUNCTION: GP4 0x0040fff2
void __cdecl FUN_0040fff2(gp4::Regs* r) {
    // 0x40ffaa picks edx from [esi+0x40]; 0x40ff8b then picks the second selector
    gp4::Regs call = *r;
    gp4::call_regs(0x0040ffaa, call);
    const uint32_t selected = call.edx;

    uint32_t edx;
    uint32_t flags;
    if (selected == GP4_GLOBAL(uint32_t, 0x00603000)) {
        gp4::call_regs(0x0040ff8b, call);
        const uint32_t reference = GP4_GLOBAL(uint32_t, 0x00602ff0);
        flags = subFlags(call.edx, reference);
        edx = (call.edx == reference) ? GP4_GLOBAL(uint32_t, 0x00603010) : GP4_GLOBAL(uint32_t, 0x0060300c);
    } else if (selected == GP4_GLOBAL(uint32_t, 0x00602ffc)) {
        gp4::call_regs(0x0040ff8b, call);
        const uint32_t reference = GP4_GLOBAL(uint32_t, 0x00602ff0);
        flags = subFlags(call.edx, reference);
        edx = (call.edx == reference) ? GP4_GLOBAL(uint32_t, 0x00603008) : GP4_GLOBAL(uint32_t, 0x00603004);
    } else {
        gp4::call_regs(0x0040ff8b, call);
        edx = call.edx;
        flags = call.eflags & 0x8d5u;
    }
    r->edx = edx;
    r->eflags = (r->eflags & ~0x8d5u) | flags;
}
GP4_IMPL(0x0040fff2, FUN_0040fff2, "regs(esi:ptr[0x48]:bytes) -> (edx, cf, pf, af, zf, sf, of); globals=fuzz:i32")
