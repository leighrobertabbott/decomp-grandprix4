#include <gp4/gp4.h>

static inline uint32_t parityFlag(uint32_t value) {
    uint32_t parity = value & 0xffu;
    parity ^= parity >> 4;
    parity ^= parity >> 2;
    parity ^= parity >> 1;
    return (parity & 1u) == 0u ? 4u : 0u;
}

// Arithmetic flags of the 32-bit `cmp left, right`.
static inline uint32_t cmpFlags32(uint32_t left, uint32_t right) {
    const uint32_t result = left - right;
    return (left < right ? 1u : 0u)
        | parityFlag(result)
        | (((left ^ right ^ result) & 0x10u) != 0u ? 0x10u : 0u)
        | (result == 0u ? 0x40u : 0u)
        | ((result & 0x80000000u) != 0u ? 0x80u : 0u)
        | (((left ^ right) & (left ^ result) & 0x80000000u) != 0u ? 0x800u : 0u);
}

// FUNCTION: GP4 0x00438007
// Stores min(0x10000 / [0x00603854], 0x7f) in byte [esi+0x1f7]. pushal/popal keep every register;
// the flags are those of `cmp eax, 0x7f` on the unclamped quotient.
void __cdecl FUN_00438007(gp4::Regs* r) {
    uint8_t* record = reinterpret_cast<uint8_t*>(static_cast<uintptr_t>(r->esi));
    uint32_t quotient = 0x10000u / GP4_GLOBAL(uint32_t, 0x00603854);
    const uint32_t flags = cmpFlags32(quotient, 0x7fu);
    if (static_cast<int32_t>(quotient) > 0x7f) {
        quotient = 0x7fu;
    }
    GP4_FIELD(uint8_t, record, 0x1f7) = static_cast<uint8_t>(quotient);
    r->eflags = (r->eflags & ~0x8d5u) | flags;
}
GP4_IMPL(0x00438007, FUN_00438007, "regs(esi:ptr[0x1ff]:bytes) -> (cf, pf, af, zf, sf, of); globals=layout=state_004125ce")
