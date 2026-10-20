#include <gp4/gp4.h>

static inline uint32_t parityFlag(uint32_t value) {
    uint32_t p = value & 0xffu;
    p ^= p >> 4;
    p ^= p >> 2;
    p ^= p >> 1;
    return (p & 1u) == 0u ? 0x04u : 0u;
}

// Flags of the 8-bit `cmp left, right`.
static inline uint32_t cmpFlags8(uint32_t left, uint32_t right) {
    left &= 0xffu;
    right &= 0xffu;
    const uint32_t result = (left - right) & 0xffu;
    return (left < right ? 0x01u : 0u) | parityFlag(result) | ((left ^ right ^ result) & 0x10u)
        | (result == 0u ? 0x40u : 0u) | (result & 0x80u)
        | ((((left ^ right) & (left ^ result)) & 0x80u) != 0u ? 0x800u : 0u);
}

// Flags of a 32-bit logic result (or / and): CF = OF = AF = 0.
static inline uint32_t logicFlags32(uint32_t result) {
    return parityFlag(result) | (result == 0u ? 0x40u : 0u) | ((result >> 24) & 0x80u);
}

// FUNCTION: GP4 0x00443148
// Picks the driver key and rolls the per-driver byte [0x0066d8c4]. With al == 0 a random value 0..63 below the count
// [0x0066d8ec] is drawn (retrying) and al becomes the list byte 0x0066d8f4[value]; ebx / ecx are left at the list base
// and that byte. Then 0x004097c7 runs for the car in esi with al as the key, and a new random byte (bit 0 forced)
// is stored in word [0x0066d8c4] unless the mode byte [0x007c2ea9] is 0x44. The flags are those of the compare with
// 0x44 or of the `or eax, 1`.
void __cdecl FUN_00443148(gp4::Regs* r) {
    gp4::Regs c = *r;
    c.st_in = 0;
    c.st_out = 0;

    if ((r->eax & 0xffu) == 0u) {
        for (;;) {
            gp4::call_regs(0x00401462, c);
            c.eax &= 0x3fu;
            if (static_cast<int32_t>(c.eax) < static_cast<int32_t>(GP4_GLOBAL(uint32_t, 0x0066d8ec))) {
                break;
            }
        }
        c.ebx = 0x0066d8f4u;
        c.ecx = GP4_ARRAY(const uint8_t, 0x0066d8f4)[c.eax];
        c.eax = (c.eax & 0xffffff00u) | c.ecx;
    }

    gp4::call_regs(0x004097c7, c);
    gp4::call_regs(0x00401462, c);
    c.eax &= 0xffu;

    const uint8_t mode = GP4_GLOBAL(uint8_t, 0x007c2ea9);
    uint32_t flags = cmpFlags8(mode, 0x44u);
    if (mode != 0x44u) {
        c.eax |= 1u;
        flags = logicFlags32(c.eax);
        GP4_GLOBAL(uint16_t, 0x0066d8c4) = static_cast<uint16_t>(c.eax);
    }

    r->eax = c.eax;
    r->ebx = c.ebx;
    r->ecx = c.ecx;
    r->eflags = (r->eflags & ~0x8d5u) | flags;
}
GP4_IMPL(0x00443148, FUN_00443148, "regs(eax:u32{0,0,0x81,1,0x16,0x96,0x41,0xff}, ebx:u32, ecx:u32, ebp:u32, esi:rec[0x66fef0,0x41c,0..21]) -> (eax, ebx, ecx, cf, pf, af, zf, sf, of); globals=layout=cars_0066fef0+list_0066fe3c+state_00412af5+state_00403800+ranges_004322a6+ring_00405a12+links_00405a12+state_004059ef+state_004094e3+div_00403bb6+trk_007ad894+circ_007adb50+lvl_007adb86+codes_0043ba12+curves_0062ecb0+wheel_00410a1c+state_0066d8c0+words_0062d210+state_0042595c+carno_0066ff6c+state_0040f112+gain_00408790+state_004087aa+div_0060138c+state_0040860a+state_00405f77+levels_006f2cc4+tbl_0066d94c+state_0043c1ba+state_0043c31b+state_0043ca8b+state_0043c4d9+tabs_0043ced2+state_00411109+scale_0066ec60+keys_00408c3e+state_00431ffb+start_004098fd+pick_00443148")
