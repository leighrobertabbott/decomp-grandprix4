#include <gp4/gp4.h>

static inline uint32_t parityFlag(uint32_t value) {
    value &= 0xffu;
    value ^= value >> 4;
    value ^= value >> 2;
    value ^= value >> 1;
    return ((value & 1u) == 0u) ? 4u : 0u;
}

// FUNCTION: GP4 0x0043e237
void __cdecl FUN_0043e237(gp4::Regs* r) {
    // imul dword [DAT_0062ed00] on eax = DAT_0062ac48 -> edx:eax ; shrd eax, edx, 1
    const int64_t product = static_cast<int64_t>(GP4_GLOBAL(int32_t, 0x0062ac48))
        * static_cast<int64_t>(GP4_GLOBAL(int32_t, 0x0062ed00));
    const uint32_t high = static_cast<uint32_t>(static_cast<uint64_t>(product) >> 32);
    const uint32_t shifted = static_cast<uint32_t>(static_cast<uint64_t>(product) >> 1);
    const uint32_t offset = GP4_GLOBAL(uint32_t, 0x00626974);
    const uint32_t result = shifted + offset;
    GP4_GLOBAL(uint16_t, 0x00626988) = static_cast<uint16_t>(result);

    const uint32_t flags = (result < shifted ? 1u : 0u)
        | parityFlag(result)
        | (((shifted ^ offset ^ result) & 0x10u) != 0u ? 0x10u : 0u)
        | (result == 0u ? 0x40u : 0u)
        | ((result & 0x80000000u) != 0u ? 0x80u : 0u)
        | ((~(shifted ^ offset) & (shifted ^ result) & 0x80000000u) != 0u ? 0x800u : 0u);
    r->eax = result;
    r->edx = high;
    r->eflags = (r->eflags & ~0x8d5u) | flags;
}
GP4_IMPL(0x0043e237, FUN_0043e237, "regs() -> (eax, edx, cf, pf, af, zf, sf, of); globals=fuzz:i32")
