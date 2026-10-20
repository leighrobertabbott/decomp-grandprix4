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
    uint8_t Unknown52[0x56];
    uint8_t FlagsA8;
    uint8_t UnknownA9[0xb7];
};

static inline Unknown_008943a0* recordAt(uint32_t address) {
    return reinterpret_cast<Unknown_008943a0*>(static_cast<uintptr_t>(address));
}

// FUNCTION: GP4 0x0043fc28
void __cdecl FUN_0043fc28(gp4::Regs* r) {
    uint32_t cursor = GP4_GLOBAL(uint32_t, 0x00627b48);
    if (cursor < GP4_GLOBAL(uint32_t, 0x00603ed0)) {
        do {
            recordAt(cursor)->Flags51 |= 0x20;
            if (GP4_GLOBAL(uint8_t, 0x00627b65) != 0) {
                recordAt(cursor)->Flags51 |= 0x08;
                recordAt(cursor)->Flags51 |= 0x40;
            }
            recordAt(cursor)->Flags51 |= 0x80;
            cursor += 0x160u;
        } while (cursor != GP4_GLOBAL(uint32_t, 0x00603ed0));
    }
    cursor = GP4_GLOBAL(uint32_t, 0x00603ed0);
    if (cursor < GP4_GLOBAL(uint32_t, 0x00627b5c)) {
        do {
            recordAt(cursor)->Flags51 |= 0x20;
            if (GP4_GLOBAL(uint8_t, 0x00627b65) != 0) {
                recordAt(cursor)->Flags51 |= 0x08;
                recordAt(cursor)->Flags51 |= 0x40;
            }
            recordAt(cursor)->Flags51 |= 0x80;
            cursor += 0x160u;
        } while (cursor != GP4_GLOBAL(uint32_t, 0x00627b5c));
    }
    cursor = GP4_GLOBAL(uint32_t, 0x00627b5c);
    if (cursor < GP4_GLOBAL(uint32_t, 0x00627b4c)) {
        do {
            recordAt(cursor)->Flags51 |= 0x20;
            recordAt(cursor)->FlagsA8 |= 0x01;
            if (GP4_GLOBAL(uint8_t, 0x00627b65) != 0) {
                recordAt(cursor)->Flags51 |= 0x08;
            }
            cursor += 0x160u;
        } while (cursor != GP4_GLOBAL(uint32_t, 0x00627b4c));
    }
    // pushal/popal restore every register; only the last "cmp edi, [0x627b4c]" flags survive.
    r->eflags = (r->eflags & ~0x8d5u) | subtractionFlags32(cursor, GP4_GLOBAL(uint32_t, 0x00627b4c));
}
GP4_IMPL(0x0043fc28, FUN_0043fc28, "regs() -> (cf, pf, af, zf, sf, of); globals=layout=ring_008943a0")
