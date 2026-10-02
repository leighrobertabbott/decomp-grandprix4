#include <gp4/gp4.h>

static inline uint32_t subtractionFlags32(uint32_t left, uint32_t right) {
    const uint32_t result = left - right;
    uint32_t parity = result & 0xffu;
    parity ^= parity >> 4;
    parity ^= parity >> 2;
    parity ^= parity >> 1;
    return (left < right ? 1u : 0u)
        | ((parity & 1u) == 0u ? 4u : 0u)
        | (((left ^ right ^ result) & 0x10u) != 0u ? 0x10u : 0u)
        | (result == 0u ? 0x40u : 0u)
        | ((result & 0x80000000u) != 0u ? 0x80u : 0u)
        | (((left ^ right) & (left ^ result) & 0x80000000u) != 0u ? 0x800u : 0u);
}

struct Unknown_008943a0 {
    uint8_t Unknown00[0x51];
    uint8_t Flags51;
    uint8_t Unknown52[0x64];
    uint8_t UnknownB6;
    uint8_t UnknownB7[0x7d];
    uint8_t Unknown134;
    uint8_t Unknown135[0x2b];
};

// FUNCTION: GP4 0x0043fcc9
void __cdecl FUN_0043fcc9(gp4::Regs* r) {
    uint32_t address = GP4_GLOBAL(uint32_t, 0x00866ecc) - 0x160u;
    uint8_t carried = reinterpret_cast<const Unknown_008943a0*>(static_cast<uintptr_t>(address))->UnknownB6;
    address -= 0x160u;
    do {
        Unknown_008943a0* entry = reinterpret_cast<Unknown_008943a0*>(static_cast<uintptr_t>(address));
        if (entry->UnknownB6 == 0 || (entry->Flags51 & 0x80) == 0) {
            entry->Unknown134 = carried;
        } else {
            carried = entry->UnknownB6;
        }
        address -= 0x160u;
    } while (address >= 0x008943a0u);
    // pushal/popal restore every register; only the final "cmp edi, 0x8943a0" flags survive.
    r->eflags = (r->eflags & ~0x8d5u) | subtractionFlags32(address, 0x008943a0u);
}
GP4_IMPL(0x0043fcc9, FUN_0043fcc9, "regs() -> (cf, pf, af, zf, sf, of); globals=layout=state_0043fcc9")
