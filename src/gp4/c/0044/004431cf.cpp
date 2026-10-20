#include <gp4/gp4.h>

static inline uint32_t parityFlag(uint32_t value) {
    uint32_t p = value & 0xffu;
    p ^= p >> 4;
    p ^= p >> 2;
    p ^= p >> 1;
    return (p & 1u) == 0u ? 0x04u : 0u;
}

// Flags of an N-bit `cmp left, right`; mask/sign select the width (0xff/0x80, 0xffff/0x8000, 0xffffffff/0x80000000).
static inline uint32_t cmpFlagsN(uint32_t left, uint32_t right, uint32_t mask, uint32_t sign) {
    left &= mask;
    right &= mask;
    const uint32_t result = (left - right) & mask;
    return (left < right ? 0x01u : 0u) | parityFlag(result) | ((left ^ right ^ result) & 0x10u)
        | (result == 0u ? 0x40u : 0u) | ((result & sign) != 0u ? 0x80u : 0u)
        | ((((left ^ right) & (left ^ result)) & sign) != 0u ? 0x800u : 0u);
}

static inline uint32_t cmpFlags8(uint32_t left, uint32_t right) { return cmpFlagsN(left, right, 0xffu, 0x80u); }
static inline uint32_t cmpFlags16(uint32_t left, uint32_t right) { return cmpFlagsN(left, right, 0xffffu, 0x8000u); }
static inline uint32_t cmpFlags32(uint32_t left, uint32_t right) { return cmpFlagsN(left, right, 0xffffffffu, 0x80000000u); }

// Flags of a 32-bit `and` / 8-bit `test` result (CF = OF = AF = 0).
static inline uint32_t logicFlags(uint32_t value, uint32_t sign) {
    return parityFlag(value) | (value == 0u ? 0x40u : 0u) | ((value & sign) != 0u ? 0x80u : 0u);
}

static inline void setFlags(gp4::Regs& c, uint32_t flags) {
    c.eflags = (c.eflags & ~0x8d5u) | flags;
}

// One step of the random "event" picker: when no event is pending (0x007c2e75 == 0), a car code is drawn through the
// 16-bit generator 0x004431b3 from the list 0x0066d8f4 (0x0066d8ec entries) and, if it passes every gate (clock
// 0x0066d8cc, counter ax against 14 and the word 0x0066d8f0, flag byte 0x0066d90a+k, level 0x0066ec4a+k + 2 below
// [0x007c2e98], 0x0043b920, and 0x004432db when [0x007c2ea9] == 0x44), handed to 0x004097c7 in al.
// All registers and flags flow from callee to callee.
// FUNCTION: GP4 0x004431cf
void __cdecl FUN_004431cf(gp4::Regs* r) {
    gp4::Regs c = *r;
    c.st_in = 0;
    c.st_out = 0;

    const uint32_t pending = GP4_GLOBAL(uint8_t, 0x007c2e75);
    setFlags(c, cmpFlags8(pending, 0u));
    if (pending != 0u) {
        *r = c;
        return;
    }

    gp4::call_regs(0x0044303b, c);
    gp4::call_regs(0x0051f1d4, c);
    if ((c.eflags & 1u) != 0u) {
        *r = c;
        return;
    }

    c.edx = GP4_GLOBAL(uint32_t, 0x0066d8cc);
    setFlags(c, cmpFlags32(c.edx, GP4_GLOBAL(uint32_t, 0x0062d238)));
    if (c.edx < GP4_GLOBAL(uint32_t, 0x0062d238)) {
        *r = c;
        return;
    }

    const int16_t counter = static_cast<int16_t>(c.eax);
    setFlags(c, cmpFlags16(c.eax, 0xeu));
    if (counter >= 0xe) {
        *r = c;
        return;
    }
    const int16_t limit = GP4_GLOBAL(int16_t, 0x0066d8f0);
    setFlags(c, cmpFlags16(c.eax, static_cast<uint32_t>(limit)));
    if (counter >= limit) {
        *r = c;
        return;
    }

    gp4::call_regs(0x004431b3, c);
    c.eax &= 0x3fu;
    const int32_t listCount = GP4_GLOBAL(int32_t, 0x0066d8ec);
    setFlags(c, cmpFlags32(c.eax, static_cast<uint32_t>(listCount)));
    if (static_cast<int32_t>(c.eax) >= listCount) {
        *r = c;
        return;
    }

    c.ebx = c.eax;
    c.ecx = GP4_ARRAY(uint8_t, 0x0066d8f4)[c.ebx];
    setFlags(c, logicFlags(c.ecx & 0xffu, 0x80u));
    if ((c.ecx & 0x80u) != 0u) {
        *r = c;
        return;
    }
    GP4_GLOBAL(uint8_t, 0x0062ad70) = static_cast<uint8_t>(c.ecx);
    c.ecx = (c.ecx & 0x3fu) - 1u;
    c.ebx = c.ecx;
    const uint32_t flagByte = *(GP4_ARRAY(uint8_t, 0x0066d90a) + static_cast<int32_t>(c.ebx));
    c.eax = (c.eax & 0xffffff00u) | flagByte;
    setFlags(c, logicFlags(flagByte, 0x80u));
    if ((flagByte & 0x80u) != 0u) {
        *r = c;
        return;
    }

    c.eax = *(GP4_ARRAY(uint8_t, 0x0066ec4a) + static_cast<int32_t>(c.ebx)) + 2u;
    const int32_t threshold = GP4_GLOBAL(int32_t, 0x007c2e98);
    setFlags(c, cmpFlags32(c.eax, static_cast<uint32_t>(threshold)));
    if (static_cast<int32_t>(c.eax) >= threshold) {
        *r = c;
        return;
    }

    gp4::call_regs(0x0043b920, c);
    if ((c.eflags & 1u) != 0u) {
        *r = c;
        return;
    }

    const uint32_t mode = GP4_GLOBAL(uint8_t, 0x007c2ea9);
    setFlags(c, cmpFlags8(mode, 0x44u));
    if (mode == 0x44u) {
        gp4::call_regs(0x004432db, c);
        if ((c.eflags & 1u) != 0u) {
            *r = c;
            return;
        }
    }

    c.eax = (c.eax & 0xffffff00u) | GP4_GLOBAL(uint8_t, 0x0062ad70);
    gp4::call_regs(0x004097c7, c);
    *r = c;
}
GP4_IMPL(0x004431cf, FUN_004431cf, "regs(eax:u32, ebx:u32, ecx:u32, edx:u32, ebp:u32, edi:u32) -> (eax, ebx, ecx, edx, esi, cf, pf, af, zf, sf, of); cases=400; globals=fuzz:i32+layout=flags_00408f7b+cars_004436f9+cars_0044303b+state_0043b920+order_00675958+flags_0066d909+keys_0044329f+list_00443278+cars_0066fef0+list_0066fe3c+state_00412af5+state_00403800+ranges_004322a6+ring_00405a12+links_00405a12+state_004059ef+state_004094e3+div_00403bb6+trk_007ad894+circ_007adb50+lvl_007adb86+codes_0043ba12+curves_0062ecb0+wheel_00410a1c+state_0066d8c0+words_0062d210+state_0042595c+carno_0066ff6c+state_0040f112+gain_00408790+state_004087aa+div_0060138c+state_0040860a+state_00405f77+levels_006f2cc4+tbl_0066d94c+state_0043c1ba+state_0043c31b+state_0043ca8b+state_0043c4d9+tabs_0043ced2+state_00411109+scale_0066ec60+keys_00408c3e+state_00431ffb+start_004098fd+keys_0044329f+gate_004431cf")
