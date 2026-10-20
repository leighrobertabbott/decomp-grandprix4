#include <gp4/gp4.h>

static inline uint32_t logicFlags8(uint8_t result) {
    uint32_t parity = result;
    parity ^= parity >> 4;
    parity ^= parity >> 2;
    parity ^= parity >> 1;
    return ((parity & 1u) == 0u ? 4u : 0u)
        | (result == 0u ? 0x40u : 0u)
        | ((result & 0x80u) != 0u ? 0x80u : 0u);
}

struct Unknown_008943a0 {
    uint8_t Unknown00[0x92];
    uint8_t Flags92;
    uint8_t Unknown93[0xcd];
};

static inline Unknown_008943a0* recordAt(uint32_t address) {
    return reinterpret_cast<Unknown_008943a0*>(static_cast<uintptr_t>(address));
}

// FUNCTION: GP4 0x0040aec5
void __cdecl FUN_0040aec5(gp4::Regs* r) {
    int32_t remaining = GP4_GLOBAL(uint16_t, 0x0062d22e);
    uint32_t cursor = GP4_GLOBAL(uint32_t, 0x00866f00);
    do {
        if (cursor < GP4_GLOBAL(uint32_t, 0x00891500)) {
            cursor -= 0x160u;
            if (cursor < GP4_GLOBAL(uint32_t, 0x00891504)) {
                cursor += GP4_GLOBAL(uint32_t, 0x00866ec4);
            }
        } else {
            cursor -= 0x160u;
            if (cursor < GP4_GLOBAL(uint32_t, 0x00891500)) {
                cursor = GP4_GLOBAL(uint32_t, 0x00891500);
            }
        }
        recordAt(cursor)->Flags92 |= 0x01;
        remaining--;
    } while (remaining >= 0);

    cursor = GP4_GLOBAL(uint32_t, 0x00866f04);
    remaining = GP4_GLOBAL(uint16_t, 0x0062d230);
    do {
        recordAt(cursor)->Flags92 |= 0x02;
        cursor += 0x160u;
        if (cursor >= GP4_GLOBAL(uint32_t, 0x00866ecc)) {
            if (cursor < GP4_GLOBAL(uint32_t, 0x00891500)) {
                cursor -= GP4_GLOBAL(uint32_t, 0x00866ec4);
            } else if (cursor >= GP4_GLOBAL(uint32_t, 0x00866ef8)) {
                cursor = GP4_GLOBAL(uint32_t, 0x00866ef8) - 0x160u;
            }
        }
        remaining--;
    } while (remaining >= 0);

    Unknown_008943a0* last = recordAt(GP4_GLOBAL(uint32_t, 0x00866ef8));
    last->Flags92 |= 0x02;
    // pushal/popal restore every register; the final "or byte [ebx+0x92], 2" leaves the logic-op flags.
    r->eflags = (r->eflags & ~0x8d5u) | logicFlags8(last->Flags92);
}
GP4_IMPL(0x0040aec5, FUN_0040aec5, "regs() -> (cf, pf, af, zf, sf, of); globals=layout=ring_0040aec5")
