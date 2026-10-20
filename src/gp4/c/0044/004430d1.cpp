#include <gp4/gp4.h>

static inline uint32_t parityFlag(uint32_t value) {
    value &= 0xffu;
    value ^= value >> 4;
    value ^= value >> 2;
    value ^= value >> 1;
    return (value & 1u) == 0u ? 4u : 0u;
}

// Flags of `test byte, 0xff`: CF = OF = AF = 0.
static inline uint32_t testFlags8(uint32_t value) {
    value &= 0xffu;
    return parityFlag(value) | (value == 0u ? 0x40u : 0u) | ((value & 0x80u) != 0u ? 0x80u : 0u);
}

// Arithmetic flags of the 8-bit `cmp left, right`.
static inline uint32_t cmpFlags8(uint32_t left, uint32_t right) {
    left &= 0xffu;
    right &= 0xffu;
    const uint32_t result = (left - right) & 0xffu;
    return (left < right ? 1u : 0u)
        | parityFlag(result)
        | (((left ^ right ^ result) & 0x10u) != 0u ? 0x10u : 0u)
        | (result == 0u ? 0x40u : 0u)
        | ((result & 0x80u) != 0u ? 0x80u : 0u)
        | (((left ^ right) & (left ^ result) & 0x80u) != 0u ? 0x800u : 0u);
}

static inline void setFlags(gp4::Regs& c, uint32_t flags) {
    c.eflags = (c.eflags & ~0x8d5u) | flags;
}

// Runs 0x00443148 for the car records at the start of the car array: with one human car ([0x007aafd4] == 0) only
// the first record (0x0066fef0, al = [0x0066d896]) is stored in [0x00679d34]; otherwise the second record
// (0x0067030c) follows, each one stored in [0x00679d34] (al = [0x0066d896]) or [0x00679d38] (al = [0x0066d87d])
// according to [0x007c2ea7] (1 puts the first record in [0x00679d38], 0 the second). One register block is
// threaded through the calls and handed back.
// FUNCTION: GP4 0x004430d1
void __cdecl FUN_004430d1(gp4::Regs* r) {
    gp4::Regs c = *r;
    c.st_in = 0;
    c.st_out = 0;

    const uint32_t twoCars = GP4_GLOBAL(uint8_t, 0x007aafd4);
    setFlags(c, testFlags8(twoCars));
    if (twoCars == 0u) {
        c.eax = (c.eax & 0xffffff00u) | GP4_GLOBAL(uint8_t, 0x0066d896);
        c.esi = 0x0066fef0u;
        GP4_GLOBAL(uint32_t, 0x00679d34) = c.esi;
        gp4::call_regs(0x00443148, c);
    } else {
        c.esi = 0x0066fef0u;
        const uint32_t firstMode = GP4_GLOBAL(uint8_t, 0x007c2ea7);
        setFlags(c, cmpFlags8(firstMode, 1u));
        if (firstMode == 1u) {
            c.eax = (c.eax & 0xffffff00u) | GP4_GLOBAL(uint8_t, 0x0066d87d);
            GP4_GLOBAL(uint32_t, 0x00679d38) = c.esi;
        } else {
            c.eax = (c.eax & 0xffffff00u) | GP4_GLOBAL(uint8_t, 0x0066d896);
            GP4_GLOBAL(uint32_t, 0x00679d34) = c.esi;
        }
        gp4::call_regs(0x00443148, c);

        c.esi = 0x0067030cu;
        const uint32_t secondMode = GP4_GLOBAL(uint8_t, 0x007c2ea7);
        setFlags(c, cmpFlags8(secondMode, 0u));
        if (secondMode == 0u) {
            c.eax = (c.eax & 0xffffff00u) | GP4_GLOBAL(uint8_t, 0x0066d87d);
            GP4_GLOBAL(uint32_t, 0x00679d38) = c.esi;
        } else {
            c.eax = (c.eax & 0xffffff00u) | GP4_GLOBAL(uint8_t, 0x0066d896);
            GP4_GLOBAL(uint32_t, 0x00679d34) = c.esi;
        }
        gp4::call_regs(0x00443148, c);
    }

    r->eax = c.eax;
    r->ebx = c.ebx;
    r->ecx = c.ecx;
    r->edx = c.edx;
    r->esi = c.esi;
    r->edi = c.edi;
    r->ebp = c.ebp;
    r->eflags = (r->eflags & ~0x8d5u) | (c.eflags & 0x8d5u);
}
GP4_IMPL(0x004430d1, FUN_004430d1, "regs(eax:u32, ebx:u32, ecx:u32, edx:u32, ebp:u32, edi:u32) -> (eax, ebx, ecx, edx, esi, edi, ebp, cf, pf, af, zf, sf, of); globals=layout=cars_0066fef0+list_0066fe3c+state_00412af5+state_00403800+ranges_004322a6+ring_00405a12+links_00405a12+state_004059ef+state_004094e3+div_00403bb6+trk_007ad894+circ_007adb50+lvl_007adb86+codes_0043ba12+curves_0062ecb0+wheel_00410a1c+state_0066d8c0+words_0062d210+state_0042595c+carno_0066ff6c+state_0040f112+gain_00408790+state_004087aa+div_0060138c+state_0040860a+state_00405f77+levels_006f2cc4+tbl_0066d94c+state_0043c1ba+state_0043c31b+state_0043ca8b+state_0043c4d9+tabs_0043ced2+state_00411109+scale_0066ec60+keys_00408c3e+state_00431ffb+start_004098fd+pick_00443148+mode_004430d1")
