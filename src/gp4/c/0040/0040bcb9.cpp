#include <gp4/gp4.h>

static inline uint32_t parityFlag(uint32_t value) {
    value &= 0xffu;
    value ^= value >> 4;
    value ^= value >> 2;
    value ^= value >> 1;
    return (value & 1u) == 0u ? 4u : 0u;
}

// DAT_00622828 = ((word 0x868558 + word 0x86855a) * DAT_00622820) >> 14 (64-bit product),
// DAT_0062282c = DAT_00622828 * DAT_00603854 (low 32 bits). pushal/popal restore every register;
// the flags that escape are those of the last "imul dword [0x603854]": CF = OF = the 64-bit
// product does not fit its low dword; PF/ZF/SF from the low dword; AF = 0 (oracle model).
// FUNCTION: GP4 0x0040bcb9
void __cdecl FUN_0040bcb9(gp4::Regs* r) {
    const uint32_t sum = static_cast<uint32_t>(GP4_GLOBAL(uint16_t, 0x00868558))
        + static_cast<uint32_t>(GP4_GLOBAL(uint16_t, 0x0086855a));
    const int64_t first = static_cast<int64_t>(static_cast<int32_t>(sum)) * GP4_GLOBAL(int32_t, 0x00622820);
    const uint32_t scaled = static_cast<uint32_t>(static_cast<uint64_t>(first) >> 14);
    GP4_GLOBAL(uint32_t, 0x00622828) = scaled;

    const int64_t second = static_cast<int64_t>(static_cast<int32_t>(scaled)) * GP4_GLOBAL(int32_t, 0x00603854);
    const uint32_t low = static_cast<uint32_t>(second);
    GP4_GLOBAL(uint32_t, 0x0062282c) = low;

    const bool overflow = second != static_cast<int64_t>(static_cast<int32_t>(low));
    const uint32_t flags = (overflow ? 0x801u : 0u)
        | parityFlag(low)
        | (low == 0u ? 0x40u : 0u)
        | ((low & 0x80000000u) != 0u ? 0x80u : 0u);
    r->eflags = (r->eflags & ~0x8d5u) | flags;
}
GP4_IMPL(0x0040bcb9, FUN_0040bcb9, "regs() -> (cf, pf, af, zf, sf, of); globals=fuzz:i32")
