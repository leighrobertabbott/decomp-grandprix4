#include <gp4/gp4.h>

static inline uint32_t parityFlag(uint32_t value) {
    uint32_t parity = value & 0xffu;
    parity ^= parity >> 4;
    parity ^= parity >> 2;
    parity ^= parity >> 1;
    return (parity & 1u) == 0u ? 4u : 0u;
}

// FUNCTION: GP4 0x00408309
void __cdecl FUN_00408309(gp4::Regs* r) {
    const uint8_t* record = reinterpret_cast<const uint8_t*>(static_cast<uintptr_t>(r->esi));
    const uint32_t halved = static_cast<uint32_t>(record[0x415]) >> 1;
    const uint32_t result = halved + 1u;
    // add eax, 1: the sum is 1..128 so neither carry nor signed overflow can occur
    const uint32_t flags = parityFlag(result)
        | (((halved ^ 1u ^ result) & 0x10u) != 0u ? 0x10u : 0u)
        | (result == 0u ? 0x40u : 0u)
        | ((result & 0x80000000u) != 0u ? 0x80u : 0u)
        | (halved == 0x7fffffffu ? 0x800u : 0u)
        | (result < halved ? 1u : 0u);
    r->eax = result;
    r->eflags = (r->eflags & ~0x8d5u) | flags;
}
GP4_IMPL(0x00408309, FUN_00408309, "regs(esi:ptr[0x41d]:bytes) -> (eax, cf, pf, af, zf, sf, of)")
