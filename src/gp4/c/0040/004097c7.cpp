#include <gp4/gp4.h>

static inline uint32_t parityFlag(uint32_t value) {
    value &= 0xffu;
    value ^= value >> 4;
    value ^= value >> 2;
    value ^= value >> 1;
    return (value & 1u) == 0u ? 4u : 0u;
}

// Flags of an 8-bit `or byte, imm` whose result is `result`: CF = OF = AF = 0.
static inline uint32_t orFlags8(uint32_t result) {
    result &= 0xffu;
    return parityFlag(result) | (result == 0u ? 0x40u : 0u) | (result & 0x80u);
}

// Puts the car in esi into a new state after 0x00409213: with byte [0x7c2e75] set it runs 0x004098fd and
// sets byte +0x74 to 1; otherwise it sets bit 3 of +0x26b, runs 0x004097fd, then sets bit 0 of +0x232
// and bit 4 of +0x25f. All registers are restored (pushal / popal); the flags are those of the last
// flag-writing instruction (the callee's on the first path, the final `or` on the second).
// FUNCTION: GP4 0x004097c7
void __cdecl FUN_004097c7(gp4::Regs* r) {
    gp4::Regs c = *r;
    c.st_in = 0;
    c.st_out = 0;
    c.st_xin_mask = 0;
    c.st_xout_mask = 0;

    gp4::call_regs(0x00409213, c);
    uint8_t* const car = reinterpret_cast<uint8_t*>(static_cast<uintptr_t>(c.esi));

    uint32_t flags;
    if (GP4_GLOBAL(uint8_t, 0x007c2e75) != 0u) {
        gp4::call_regs(0x004098fd, c);
        car[0x74] = 1;
        flags = c.eflags;
    } else {
        car[0x26b] = static_cast<uint8_t>(car[0x26b] | 8u);
        gp4::call_regs(0x004097fd, c);
        car[0x232] = static_cast<uint8_t>(car[0x232] | 1u);
        car[0x25f] = static_cast<uint8_t>(car[0x25f] | 0x10u);
        flags = orFlags8(car[0x25f]);
    }

    r->eflags = (r->eflags & ~0x8d5u) | (flags & 0x8d5u);
}
GP4_IMPL(0x004097c7, FUN_004097c7, "regs(eax:u32, ebx:u32, ebp:u32, esi:rec[0x66fef0,0x41c,0..21]) -> (cf, pf, af, zf, sf, of); globals=layout=cars_0066fef0+list_0066fe3c+state_00412af5+state_00403800+ranges_004322a6+ring_00405a12+links_00405a12+state_004059ef+state_004094e3+div_00403bb6+trk_007ad894+circ_007adb50+lvl_007adb86+codes_0043ba12+curves_0062ecb0+wheel_00410a1c+state_0066d8c0+words_0062d210+state_0042595c+carno_0066ff6c+state_0040f112+gain_00408790+state_004087aa+div_0060138c+state_0040860a+state_00405f77+levels_006f2cc4+tbl_0066d94c+state_0043c1ba+state_0043c31b+state_0043ca8b+state_0043c4d9+tabs_0043ced2+state_00411109+scale_0066ec60+keys_00408c3e+state_00431ffb+start_004098fd")
