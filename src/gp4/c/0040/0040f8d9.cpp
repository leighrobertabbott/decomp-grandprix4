#include <gp4/gp4.h>

static inline uint32_t parityFlag(uint32_t value) {
    value &= 0xffu;
    value ^= value >> 4;
    value ^= value >> 2;
    value ^= value >> 1;
    return (value & 1u) == 0u ? 4u : 0u;
}

// Flags of a byte / word / dword logic result (test): CF = OF = AF = 0.
static inline uint32_t logicFlags(uint32_t result, uint32_t signBit) {
    return parityFlag(result)
        | (result == 0u ? 0x40u : 0u)
        | ((result & signBit) != 0u ? 0x80u : 0u);
}

// Flags of the 32-bit `cmp left, right`.
static inline uint32_t cmpFlags32(uint32_t left, uint32_t right) {
    const uint32_t result = left - right;
    return (left < right ? 1u : 0u)
        | parityFlag(result)
        | (((left ^ right ^ result) & 0x10u) != 0u ? 0x10u : 0u)
        | (result == 0u ? 0x40u : 0u)
        | ((result & 0x80000000u) != 0u ? 0x80u : 0u)
        | (((left ^ right) & (left ^ result) & 0x80000000u) != 0u ? 0x800u : 0u);
}

// FUNCTION: GP4 0x0040f8d9
// For the player's car (esi = [0x679d30], only when it equals [0x679d2c] and [0x66d7a4] is 0):
// with bit 9 of [0x66d8b4] set the 16-byte entry of the table at 0x6269b8 selected by car byte
// +0x234 supplies the two words [0x602d84] / [0x602d88] (+8 / +0xc) and the sign-extended word
// [0x602d7c] (+0), 0x004056c9 computes eax, which is scaled down by 256; otherwise ax = 0x7800.
// Then 0x0040f833 runs with that eax and esi. pushal/popal restore every register; the flags are
// those of the last flag writer on each path.
void __cdecl FUN_0040f8d9(gp4::Regs* r) {
    const uint32_t disabled = GP4_GLOBAL(uint8_t, 0x0066d7a4);
    if (disabled != 0u) {
        r->eflags = (r->eflags & ~0x8d5u) | logicFlags(disabled, 0x80u);
        return;
    }

    const uint32_t car = GP4_GLOBAL(uint32_t, 0x00679d30);
    const uint32_t player = GP4_GLOBAL(uint32_t, 0x00679d2c);
    if (car != player) {
        r->eflags = (r->eflags & ~0x8d5u) | cmpFlags32(car, player);
        return;
    }

    gp4::Regs c = *r;
    c.st_in = 0;
    c.st_out = 0;
    c.esi = car;
    c.eax = (c.eax & 0xffff0000u) | 0x7800u;

    const uint32_t options = GP4_GLOBAL(uint16_t, 0x0066d8b4) & 0x200u;
    c.eflags = (c.eflags & ~0x8d5u) | logicFlags(options, 0x8000u);
    if (options != 0u) {
        const uint32_t index = GP4_FIELD(uint8_t, car, 0x234);
        const uint8_t* entry = GP4_ARRAY(uint8_t, 0x006269b8) + index * 0x10u;
        c.ebx = static_cast<uint32_t>(reinterpret_cast<uintptr_t>(entry));
        GP4_GLOBAL(uint32_t, 0x00602d84) = *reinterpret_cast<const uint32_t*>(entry + 8);
        GP4_GLOBAL(uint32_t, 0x00602d88) = *reinterpret_cast<const uint32_t*>(entry + 0xc);
        GP4_GLOBAL(int32_t, 0x00602d7c) = *reinterpret_cast<const int16_t*>(entry);
        c.eax = static_cast<uint32_t>(*reinterpret_cast<const int16_t*>(entry));
        gp4::call_regs(0x004056c9, c);
        c.eax = static_cast<uint32_t>(static_cast<int32_t>(c.eax) >> 8);
    }
    gp4::call_regs(0x0040f833, c);

    r->eflags = (r->eflags & ~0x8d5u) | (c.eflags & 0x8d5u);
}
GP4_IMPL(0x0040f8d9, FUN_0040f8d9, "regs() -> (cf, pf, af, zf, sf, of); globals=fuzz:i32+layout=carbg_0066fef0+order_00418f8f+state_0040f898+engine_00413606+state_00404ed0+state_00404f33+drive_004054e6+lanes_00429fcd+state_0041242f+player_0040f8d9")
