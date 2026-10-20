#include <gp4/gp4.h>

static inline uint32_t parityFlag(uint32_t value) {
    uint32_t parity = value & 0xffu;
    parity ^= parity >> 4;
    parity ^= parity >> 2;
    parity ^= parity >> 1;
    return (parity & 1u) == 0u ? 4u : 0u;
}

// Flags of the 32-bit `cmp left, right`.
static inline uint32_t cmpFlags32(uint32_t left, uint32_t right) {
    const uint32_t result = left - right;
    return (left < right ? 1u : 0u)
        | parityFlag(result)
        | (((left ^ right ^ result) & 0x10u) != 0u ? 0x10u : 0u)
        | (result == 0u ? 0x40u : 0u)
        | ((result & 0x80000000u) != 0u ? 0x80u : 0u)
        | ((((left ^ right) & (left ^ result)) & 0x80000000u) != 0u ? 0x800u : 0u);
}

// FUNCTION: GP4 0x004452f1
// Opcode dispatch: the low byte of eax (0..14) jumps through the table at 0x00601270 to the opcode handler,
// which returns to the caller; larger values return at once. Flags are those of `cmp eax, 0xe`.
void __cdecl FUN_004452f1(gp4::Regs* r) {
    const uint32_t opcode = r->eax & 0xffu;
    const uint32_t flags = cmpFlags32(opcode, 0xeu);
    r->eax = opcode;
    r->eflags = (r->eflags & ~0x8d5u) | flags;
    if (opcode > 0xeu) {
        return;
    }

    gp4::Regs c = *r;
    c.st_in = 0;
    c.st_out = 0;
    gp4::call_regs(GP4_ARRAY(uint32_t, 0x00601270)[opcode], c);

    r->eax = c.eax;
    r->ebx = c.ebx;
    r->ecx = c.ecx;
    r->edx = c.edx;
    r->esi = c.esi;
    r->edi = c.edi;
    r->ebp = c.ebp;
    r->eflags = (r->eflags & ~0x8d5u) | (c.eflags & 0x8d5u);
}
GP4_IMPL(0x004452f1, FUN_004452f1, "regs(eax:u32{0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,0x10e,0xffffff05}, edx:u32) -> (eax, ebx, ecx, edx, esi, edi, ebp, cf, pf, af, zf, sf, of); globals=layout=carbg_0066fef0+state_0040fa01+state_0041104f+ctl_007ab590+carst_0040f6d0+state_004320f2+stream_0062d1a0+trackgeo_008943a0+tbl_00868598+cars_0066fef0+list_0066fe3c+state_00412af5+state_00403800+div_00403bb6+state_0040f629+lanes_0040da62+pend_0040daeb+state_004094e3+ranges_004322a6+state_0040705f+frac_0040705f+state_00407af6+state_004381ab+state_00406b0a+ring_00406ac7+bounds_00406a57+ring_00405a12+links_00405a12+gridlinks_008943a0+pit_00406dbb+segflag_004381ab+state_0040759e+trackfar_008943a0+state_004059ef+trk_007ad894+circ_007adb50+lvl_007adb86+codes_0043ba12+curves_0062ecb0+wheel_00410a1c+state_0066d8c0+words_0062d210+state_0042595c+carno_0066ff6c+state_0040f112+gain_00408790+state_004087aa+div_0060138c+state_0040860a+state_00405f77+levels_006f2cc4+tbl_0066d94c+state_0043c1ba+state_0043c31b+state_0043ca8b+state_0043c4d9+tabs_0043ced2+state_00411109+counter_0040f0dd+flags_0066d909+scale_0066ec60+keys_00408c3e+stream_0062d1a8+flag_007a97a0+cars_00444d60+cur_00444c9a+stream_00444ff1")
