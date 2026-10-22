#include <gp4/gp4.h>
#include <gp4/flags.h>

// Per-frame step of the car simulation (the 0x80 mode branch of 0x0040dfb8): the first hook is
// 0x0044236c when byte [0x7c2e6c] is set, else 0x004422dc; then 0x004091ef, 0x004424d5, 0x004091b5,
// 0x00408d6d, 0x00409b87 and 0x00410d04, and with [0x7c2e6c] set also 0x00409c13 and 0x00410d4b.
// Every callee sees the registers the previous one left (no saves in between); the registers and
// flags returned are those of the last callee. Its only caller runs 0x0040860a first, so the
// per-car arrays that routine resets are given in that state.
static __declspec(noinline) void firstHookOff(gp4::Regs& c) { gp4::call_regs(0x004422dc, c); }
static __declspec(noinline) void firstHookOn(gp4::Regs& c) { gp4::call_regs(0x0044236c, c); }

// FUNCTION: GP4 0x0040df1c
void __cdecl FUN_0040df1c(gp4::Regs* r) {
    gp4::Regs c = *r;
    c.st_in = 0;
    c.st_out = 0;

    if (GP4_GLOBAL(uint8_t, 0x007c2e6c) == 0u) {
        firstHookOff(c);
    } else {
        firstHookOn(c);
    }
    gp4::call_regs(0x004091ef, c);
    gp4::call_regs(0x004424d5, c);
    gp4::call_regs(0x004091b5, c);
    gp4::call_regs(0x00408d6d, c);
    gp4::call_regs(0x00409b87, c);
    gp4::call_regs(0x00410d04, c);
    const uint8_t dual = GP4_GLOBAL(uint8_t, 0x007c2e6c);
    if (dual != 0u) {
        gp4::call_regs(0x00409c13, c);
        gp4::call_regs(0x00410d4b, c);
    } else {
        // `cmp byte [0x7c2e6c], 0; je ret`: the compare is the last flag writer
        c.eflags = gp4::flags::merge(c.eflags, gp4::flags::sub8(dual, 0u));
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
GP4_IMPL(0x0040df1c, FUN_0040df1c, "regs(edx:u32, ebp:u32) -> (eax, ebx, ecx, edx, esi, edi, ebp, cf, pf, af, zf, sf, of); maxinsn=80000000; wall=3600; globals=layout=setup_007adb54+trackgeo_008943a0+tbl_00868598+state_0040d8a5+state_0041104f+flag_007a97a0+cars_0066fef0+list_0066fe3c+lanes_0040da62+pend_0040daeb+state_00412af5+state_00403800+div_00403bb6+state_0040f629+ranges_004322a6+state_0040705f+frac_0040705f+state_00406b0a+ring_00406ac7+bounds_00406a57+ring_00405a12+links_00405a12+pit_00406dbb+state_0040759e+trackfar_008943a0+segflag_004381ab+ctl_007ab548+ctl_007ab590+carst_0040f6d0+carf_00410d04+state_004059ef+state_004094e3+trk_007ad894+circ_007adb50+lvl_007adb86+codes_0043ba12+wheel_00410a1c+state_0066d8c0+words_0062d210+state_0042595c+carno_0066ff6c+state_0040f112+gain_00408790+state_004087aa+div_0060138c+state_0040860a+state_00405f77+levels_006f2cc4+tbl_0066d94c+state_0043c1ba+state_0043c31b+state_0043ca8b+state_0043c4d9+tabs_0043ced2+state_00411109+scale_0066ec60+keys_00408c3e+state_00431ffb+start_004098fd+pick_00443148+mode_004430d1+grid_00408d6d+order_00675958+search_0066d82c+state_004121c7+state_004120e4+state_00411f9f+state_004123a6+frac_00624790+state_00416708+state_0040dfe9+flags_00408f7b+lap_0043b482+state_0044334d+state_004320f2+neg22_00520023; gfields={0x0066d90a*22/1:u8{=0}, 0x0066fb80*22/4:u32{=0x21980000}, 0x0066fbd8*22/4:u32{=0x21980000}, 0x0066fc30*22/4:u32{=0x21980000}, 0x0066fc88*22/4:u32{=0x21980000}, 0x0066ec4a*22/1:u8{=0}, 0x0066d920*22/1:u8{=0,2,0x40,0x80,0xfe}, 0x00601d68:i32[0x100000..0x7fffffff], 0x00601d8c*4/4:i32[16..32000]}")
