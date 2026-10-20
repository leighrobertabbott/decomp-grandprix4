#include <gp4/gp4.h>

static inline uint32_t parityFlag(uint32_t value) {
    uint32_t parity = value & 0xffu;
    parity ^= parity >> 4;
    parity ^= parity >> 2;
    parity ^= parity >> 1;
    return (parity & 1u) == 0u ? 4u : 0u;
}

// FUNCTION: GP4 0x0043c4d9
void __cdecl FUN_0043c4d9(gp4::Regs* r) {
    // Stores the rounded minute count from 0x43c4c1, clamped to 255, at [esi + 0x233].
    // eax is saved around the call; the flags left behind are those of "cmp eax, 0xff".
    uint8_t* const body = reinterpret_cast<uint8_t*>(static_cast<uintptr_t>(r->esi));

    gp4::Regs callee = *r;
    gp4::call_regs(0x0043c4c1, callee);

    const uint32_t left = callee.eax;
    const uint32_t right = 0xffu;
    const uint32_t result = left - right;
    const uint32_t flags = parityFlag(result)
        | (left < right ? 1u : 0u)
        | (((left ^ right ^ result) & 0x10u) != 0u ? 0x10u : 0u)
        | (result == 0u ? 0x40u : 0u)
        | ((result & 0x80000000u) != 0u ? 0x80u : 0u)
        | (((left ^ right) & (left ^ result) & 0x80000000u) != 0u ? 0x800u : 0u);

    int32_t minutes = static_cast<int32_t>(left);
    if (minutes > 0xff) {
        minutes = 0xff;
    }
    GP4_FIELD(uint8_t, body, 0x233) = static_cast<uint8_t>(minutes);

    r->eflags = (r->eflags & ~0x8d5u) | flags;
}
GP4_IMPL(0x0043c4d9, FUN_0043c4d9, "regs(esi:ptr[0x240]:bytes) -> (cf, pf, af, zf, sf, of); globals=layout=state_0043c4d9")
