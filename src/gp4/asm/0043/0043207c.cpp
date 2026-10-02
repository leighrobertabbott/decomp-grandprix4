#include <gp4/gp4.h>

static inline uint32_t parityFlag(uint32_t value) {
    uint32_t parity = value & 0xffu;
    parity ^= parity >> 4;
    parity ^= parity >> 2;
    parity ^= parity >> 1;
    return (parity & 1u) == 0u ? 4u : 0u;
}

// FUNCTION: GP4 0x0043207c
void __cdecl FUN_0043207c(gp4::Regs* r) {
    const uint8_t* record = reinterpret_cast<const uint8_t*>(static_cast<uintptr_t>(r->esi));
    // movzx eax, [esi+0xfd] ; imul eax, eax, 0x420 ; add eax, [0x866f08] ; add eax, 0x160
    const uint32_t scaled = static_cast<uint32_t>(record[0xfd]) * 0x420u;
    const uint32_t base = scaled + GP4_GLOBAL(uint32_t, 0x00866f08);
    const uint32_t result = base + 0x160u;
    // the flags are those of the final add
    const uint32_t flags = (result < base ? 1u : 0u)
        | parityFlag(result)
        | (((base ^ 0x160u ^ result) & 0x10u) != 0u ? 0x10u : 0u)
        | (result == 0u ? 0x40u : 0u)
        | ((result & 0x80000000u) != 0u ? 0x80u : 0u)
        | ((~(base ^ 0x160u) & (base ^ result) & 0x80000000u) != 0u ? 0x800u : 0u);
    r->eax = result;
    r->eflags = (r->eflags & ~0x8d5u) | flags;
}
GP4_IMPL(0x0043207c, FUN_0043207c, "regs(esi:ptr[0x105]:bytes) -> (eax, cf, pf, af, zf, sf, of); globals=fuzz:i32")
