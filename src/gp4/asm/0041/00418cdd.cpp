#include <gp4/gp4.h>

static inline uint32_t parityFlag8(uint32_t value) {
    uint32_t parity = value & 0xffu;
    parity ^= parity >> 4;
    parity ^= parity >> 2;
    parity ^= parity >> 1;
    return (parity & 1u) == 0u ? 4u : 0u;
}

static inline uint32_t comparisonFlags32(uint32_t left, uint32_t right) {
    const uint32_t result = left - right;
    return (left < right ? 1u : 0u)
        | parityFlag8(result)
        | (((left ^ right ^ result) & 0x10u) != 0u ? 0x10u : 0u)
        | (result == 0u ? 0x40u : 0u)
        | ((result & 0x80000000u) != 0u ? 0x80u : 0u)
        | (((left ^ right) & (left ^ result) & 0x80000000u) != 0u ? 0x800u : 0u);
}

// Flags left by a 32-bit OR/AND: CF = OF = AF = 0, PF/ZF/SF from the result.
static inline uint32_t logicFlags32(uint32_t result) {
    return parityFlag8(result)
        | (result == 0u ? 0x40u : 0u)
        | ((result & 0x80000000u) != 0u ? 0x80u : 0u);
}

// FUNCTION: GP4 0x00418cdd
// (dead tests in the original: cmp eax,0 after mov eax,-1 never branches)
// eax and edx are two record pointers; the status word of the entry selected by eax's record gets
// bit 0x200 set, and bit 0x400 too when either pointer equals the global at 0x00679d30.
void __cdecl FUN_00418cdd(gp4::Regs* r) {
    const uint32_t first = r->eax;
    const uint32_t second = r->edx;

    gp4::Regs call = *r;
    call.esi = first;
    call.ebx = second;
    gp4::call_regs(0x00418901, call);
    uint32_t* status = reinterpret_cast<uint32_t*>(static_cast<uintptr_t>(call.eax));

    *status = *status & 0xfffffdffu;
    *status = *status | 0x200u;

    const uint32_t selected = GP4_GLOBAL(uint32_t, 0x00679d30);
    uint32_t flags;
    if (second == selected || first == selected) {
        *status = *status & 0xfffffbffu;
        *status = *status | 0x400u;
        flags = logicFlags32(*status);
    } else {
        flags = comparisonFlags32(first, selected);
    }

    r->eflags = (r->eflags & ~0x8d5u) | flags;
}
GP4_IMPL(0x00418cdd, FUN_00418cdd, "regs(eax:ptr[0x20a]:layout=rec_00418cdd, edx:u32[0..1]) -> (cf, pf, af, zf, sf, of); globals=layout=state_00418cdd")
