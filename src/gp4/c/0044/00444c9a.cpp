#include <gp4/gp4.h>

enum : uint32_t { kArithFlags = 0x8d5u };

// FUNCTION: GP4 0x00444c9a
// Opcode 3 handler (jump-table target of the stream dispatcher 0x004452f1). esi = the car record
// [0x00679d38]. The three words 0x0066d8be / c0 / c2 are saved on the stack (the high half of each pushed
// dword is the incoming eax) while 0x00442c41, 0x00408742 run; byte +1 of the stream at the read cursor
// [0x0062d1a8] goes to 0x0066d87d; then 0x004097c7, 0x00445130 run, the car record (esi as 0x00445130
// leaves it) gets +0x83 |= 0x20 and +0xe5 &= 0xfb, 0x0040f6d0 runs and the three words are restored. One
// register block is threaded through the callees and handed back; eax is the first pushed dword.
void __cdecl FUN_00444c9a(gp4::Regs* r) {
    gp4::Regs s = *r;
    s.st_in = 0;
    s.st_out = 0;
    s.st_xin_mask = 0;
    s.st_xout_mask = 0;

    s.esi = GP4_GLOBAL(uint32_t, 0x00679d38);
    const uint32_t high = s.eax & 0xffff0000u;
    const uint32_t saved0 = high | GP4_GLOBAL(uint16_t, 0x0066d8be);
    const uint32_t saved1 = high | GP4_GLOBAL(uint16_t, 0x0066d8c0);
    const uint32_t saved2 = high | GP4_GLOBAL(uint16_t, 0x0066d8c2);
    s.eax = saved2;

    gp4::call_regs(0x00442c41, s);
    gp4::call_regs(0x00408742, s);

    s.ebx = GP4_GLOBAL(uint32_t, 0x0062d1a8);
    const uint8_t opcodeByte = GP4_FIELD(uint8_t, s.ebx, 1);
    s.eax = (s.eax & 0xffffff00u) | opcodeByte;
    GP4_GLOBAL(uint8_t, 0x0066d87d) = opcodeByte;

    gp4::call_regs(0x004097c7, s);
    gp4::call_regs(0x00445130, s);

    GP4_FIELD(uint8_t, s.esi, 0x83) = static_cast<uint8_t>(GP4_FIELD(uint8_t, s.esi, 0x83) | 0x20u);
    GP4_FIELD(uint8_t, s.esi, 0xe5) = static_cast<uint8_t>(GP4_FIELD(uint8_t, s.esi, 0xe5) & 0xfbu);

    gp4::call_regs(0x0040f6d0, s);

    GP4_GLOBAL(uint16_t, 0x0066d8c2) = static_cast<uint16_t>(saved2);
    GP4_GLOBAL(uint16_t, 0x0066d8c0) = static_cast<uint16_t>(saved1);
    GP4_GLOBAL(uint16_t, 0x0066d8be) = static_cast<uint16_t>(saved0);

    r->eax = saved0;
    r->ebx = s.ebx;
    r->ecx = s.ecx;
    r->edx = s.edx;
    r->esi = s.esi;
    r->edi = s.edi;
    r->ebp = s.ebp;
    r->eflags = (r->eflags & ~kArithFlags) | (s.eflags & kArithFlags);
}
GP4_IMPL(0x00444c9a, FUN_00444c9a, "regs(eax:u32) -> (eax, ebx, ecx, edx, esi, edi, ebp, cf, pf, af, zf, sf, of); globals=fuzz:i32+layout=trackgeo_008943a0+tbl_00868598+carbg_0066fef0+cars_0066fef0+list_0066fe3c+state_00412af5+state_00403800+div_00403bb6+state_0040f629+lanes_0040da62+pend_0040daeb+state_004094e3+ranges_004322a6+state_0040705f+frac_0040705f+state_00407af6+state_004381ab+state_00406b0a+ring_00406ac7+bounds_00406a57+ring_00405a12+links_00405a12+gridlinks_008943a0+pit_00406dbb+segflag_004381ab+state_0040759e+trackfar_008943a0+state_004059ef+trk_007ad894+circ_007adb50+lvl_007adb86+codes_0043ba12+curves_0062ecb0+wheel_00410a1c+state_0066d8c0+words_0062d210+state_0042595c+carno_0066ff6c+state_0040f112+gain_00408790+state_004087aa+div_0060138c+state_0040860a+state_00405f77+levels_006f2cc4+tbl_0066d94c+state_0043c1ba+state_0043c31b+state_0043ca8b+state_0043c4d9+tabs_0043ced2+state_00411109+counter_0040f0dd+flags_0066d909+scale_0066ec60+keys_00408c3e+stream_0062d1a8+flag_007a97a0+cars_00444d60+cur_00444c9a")
