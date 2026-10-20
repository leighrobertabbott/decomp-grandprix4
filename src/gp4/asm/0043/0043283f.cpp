#include <gp4/gp4.h>

static inline uint32_t parityFlag(uint32_t value) {
    uint32_t parity = value & 0xffu;
    parity ^= parity >> 4;
    parity ^= parity >> 2;
    parity ^= parity >> 1;
    return (parity & 1u) == 0u ? 4u : 0u;
}

// Flags of an 8-bit `cmp a, b`.
static inline uint32_t cmpFlags8(uint32_t a8, uint32_t b8) {
    const uint32_t a = a8 & 0xffu;
    const uint32_t b = b8 & 0xffu;
    const uint32_t result = (a - b) & 0xffu;
    return (a < b ? 1u : 0u)
        | parityFlag(result)
        | (((a ^ b ^ result) & 0x10u) != 0u ? 0x10u : 0u)
        | (result == 0u ? 0x40u : 0u)
        | ((result & 0x80u) != 0u ? 0x80u : 0u)
        | ((((a ^ b) & (a ^ result)) & 0x80u) != 0u ? 0x800u : 0u);
}

// FUNCTION: GP4 0x0043283f
// Car (esi) start-of-race / reset step, pushal ... popal: every register is preserved and the flags
// are those of the last flag writer. Clears +0xcd bit 1, runs 0x432436 and 0x432933, clears +0x25d and
// +0xff bit 0. Mode byte [0x007c2ea9] negative: 0x43c4f3, 0x44126d, 0x408ab4, +0x7b |= 0x10 and, for
// the car in [0x00679d2c], [0x0066d882] = [0x0088f6a8] and [0x0088f6b0] = 0xffff. Otherwise +0x8b |= 0x10,
// +0x220 = [0x0066d814] + 0xfa0, 0x432654, 0x441304, 0x408892 and, with mode bit 6, +0x7b |= 0x20 and
// [0x0066d886] = [0x0088f6bc] for the car in [0x00679d2c]. +0x82 bit 0 is set (unless +0xe5 bit 2) for
// the car in [0x0066d82c] (when [0x007c2e6c] is nonzero) or in both [0x0066d820] and [0x00679d2c].
// 0x43247d runs next and 0x432799 follows when the mode byte is exactly 0x80; the flags are those
// of that callee, or of "cmp byte [0x007c2ea9], 0x80".
void __cdecl FUN_0043283f(gp4::Regs* r) {
    uint8_t* car = reinterpret_cast<uint8_t*>(static_cast<uintptr_t>(r->esi));

    gp4::Regs w = *r;
    w.st_in = 0;
    w.st_out = 0;
    w.st_xin_mask = 0;
    w.st_xout_mask = 0;

    GP4_FIELD(uint8_t, car, 0xcd) &= 0xfdu;
    gp4::call_regs(0x00432436, w);
    gp4::call_regs(0x00432933, w);
    GP4_FIELD(uint8_t, car, 0x25d) = 0u;
    GP4_FIELD(uint8_t, car, 0xff) &= 0xfeu;

    if ((GP4_GLOBAL(uint8_t, 0x007c2ea9) & 0x80u) != 0u) {
        gp4::call_regs(0x0043c4f3, w);
        gp4::call_regs(0x0044126d, w);
        gp4::call_regs(0x00408ab4, w);
        GP4_FIELD(uint8_t, car, 0x7b) |= 0x10u;
        if (r->esi == GP4_GLOBAL(uint32_t, 0x00679d2c)) {
            GP4_GLOBAL(uint16_t, 0x0066d882) = GP4_GLOBAL(uint16_t, 0x0088f6a8);
            GP4_GLOBAL(uint16_t, 0x0088f6b0) = 0xffffu;
        }
    } else {
        GP4_FIELD(uint8_t, car, 0x8b) |= 0x10u;
        GP4_FIELD(uint32_t, car, 0x220) = GP4_GLOBAL(uint32_t, 0x0066d814);
        GP4_FIELD(uint32_t, car, 0x220) += 0xfa0u;
        gp4::call_regs(0x00432654, w);
        gp4::call_regs(0x00441304, w);
        gp4::call_regs(0x00408892, w);
        if ((GP4_GLOBAL(uint8_t, 0x007c2ea9) & 0x40u) != 0u) {
            GP4_FIELD(uint8_t, car, 0x7b) |= 0x20u;
            if (r->esi == GP4_GLOBAL(uint32_t, 0x00679d2c)) {
                GP4_GLOBAL(uint16_t, 0x0066d886) = GP4_GLOBAL(uint16_t, 0x0088f6bc);
            }
        }
    }

    if ((GP4_FIELD(uint8_t, car, 0xe5) & 4u) == 0u) {
        if ((GP4_GLOBAL(uint8_t, 0x007c2e6c) != 0u && r->esi == GP4_GLOBAL(uint32_t, 0x0066d82c))
            || (r->esi == GP4_GLOBAL(uint32_t, 0x0066d820) && r->esi == GP4_GLOBAL(uint32_t, 0x00679d2c))) {
            GP4_FIELD(uint8_t, car, 0x82) |= 1u;
        }
    }

    gp4::call_regs(0x0043247d, w);
    uint32_t flags;
    if (GP4_GLOBAL(uint8_t, 0x007c2ea9) == 0x80u) {
        gp4::call_regs(0x00432799, w);
        flags = w.eflags & 0x8d5u;
    } else {
        flags = cmpFlags8(GP4_GLOBAL(uint8_t, 0x007c2ea9), 0x80u);
    }

    r->eflags = (r->eflags & ~0x8d5u) | flags;
}
GP4_IMPL(0x0043283f, FUN_0043283f, "regs(eax:u32, edx:u32, esi:ptr[0x420]:layout=rec_0043259c+car_00431ffb+rec_00410894+rec_004087aa) -> (cf, pf, af, zf, sf, of); globals=fuzz:i32+layout=state_0066d8c0+state_00410894+state_004087aa+trk_007ad894+circ_007adb50+codes_0043ba12+curves_0062ecb0+div_0060138c+state_0043c1ba+state_0043c4d9+state_004088c2+state_00408998+counter_0040f0dd+state_0043260e+state_004327cd+state_00432372+temp_00602b50+state_0040edc3+state_0040efa2")
