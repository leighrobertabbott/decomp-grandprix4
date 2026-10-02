#include <gp4/gp4.h>

static inline uint32_t parityFlag(uint32_t value) {
    uint32_t parity = value & 0xffu;
    parity ^= parity >> 4;
    parity ^= parity >> 2;
    parity ^= parity >> 1;
    return (parity & 1u) == 0u ? 4u : 0u;
}

// FUNCTION: GP4 0x0043d24e
void __cdecl FUN_0043d24e(gp4::Regs* r) {
    const uint32_t body = r->esi;
    const uint16_t value = GP4_FIELD(uint16_t, reinterpret_cast<void*>(static_cast<uintptr_t>(body)), 0x9a);

    // cmp ax, 0x4000 (16-bit)
    const uint32_t result = static_cast<uint32_t>(static_cast<uint16_t>(value - 0x4000u));
    uint32_t flags = parityFlag(result)
        | (value < 0x4000u ? 1u : 0u)
        | (((value ^ 0x4000u ^ result) & 0x10u) != 0u ? 0x10u : 0u)
        | (result == 0u ? 0x40u : 0u)
        | ((result & 0x8000u) != 0u ? 0x80u : 0u)
        | ((((value ^ 0x4000u) & (value ^ result)) & 0x8000u) != 0u ? 0x800u : 0u);

    // jl skips "or ax, ax" (signed less: SF != OF)
    const bool less = ((flags & 0x80u) != 0u) != ((flags & 0x800u) != 0u);
    if (!less) {
        // or ax, ax: CF = OF = AF = 0, PF/ZF/SF from the value
        flags = parityFlag(value)
            | (value == 0u ? 0x40u : 0u)
            | ((value & 0x8000u) != 0u ? 0x80u : 0u);
    }

    r->eflags = (r->eflags & ~0x8d5u) | flags;
}
GP4_IMPL(0x0043d24e, FUN_0043d24e, "regs(esi:ptr[0xa2]:bytes) -> (cf, pf, af, zf, sf, of)")
