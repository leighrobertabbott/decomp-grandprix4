#include <gp4/gp4.h>
#include <gp4/flags.h>

// FUNCTION: GP4 0x0040df63
// Car simulation chain (the sibling of 0x0040df1c): 0x00442fa3, 0x004422dc, 0x004091ef, 0x00442974,
// 0x004091b5, 0x00408d6d, 0x004430d1, 0x00409b87 and 0x00410d04 run in turn with the registers each
// leaves behind. When byte 0x007c2e75 is nonzero and the car pointer [0x0066d820] is not null, bit 1
// of its byte +0x7b and bit 5 of its byte +0xfe are set. The flags are those of the last flag writer
// on the path taken (the byte compare, `or esi, esi` or the final `or byte [esi+0xfe], 0x20`).
void __cdecl FUN_0040df63(gp4::Regs* r) {
    gp4::Regs c = *r;
    c.st_in = 0;
    c.st_out = 0;
    // `cmp byte [0x7c2e6c], 0; je next`: both outcomes fall through to the chain
    c.eflags = gp4::flags::merge(c.eflags, gp4::flags::sub8(GP4_GLOBAL(uint8_t, 0x007c2e6c), 0u));

    gp4::call_regs(0x00442fa3, c);
    gp4::call_regs(0x004422dc, c);
    gp4::call_regs(0x004091ef, c);
    gp4::call_regs(0x00442974, c);
    gp4::call_regs(0x004091b5, c);
    gp4::call_regs(0x00408d6d, c);
    gp4::call_regs(0x004430d1, c);
    gp4::call_regs(0x00409b87, c);
    gp4::call_regs(0x00410d04, c);

    const uint8_t mode = GP4_GLOBAL(uint8_t, 0x007c2e75);
    uint32_t flags = gp4::flags::sub8(mode, 0u);
    if (mode != 0u) {
        c.esi = GP4_GLOBAL(uint32_t, 0x0066d820);
        flags = gp4::flags::logic32(c.esi);
        if (c.esi != 0u) {
            uint8_t* car = reinterpret_cast<uint8_t*>(static_cast<uintptr_t>(c.esi));
            GP4_FIELD(uint8_t, car, 0x7b) |= 2u;
            const uint8_t f = static_cast<uint8_t>(GP4_FIELD(uint8_t, car, 0xfe) | 0x20u);
            GP4_FIELD(uint8_t, car, 0xfe) = f;
            flags = gp4::flags::logic8(f);
        }
    }

    r->eax = c.eax;
    r->ebx = c.ebx;
    r->ecx = c.ecx;
    r->edx = c.edx;
    r->esi = c.esi;
    r->edi = c.edi;
    r->ebp = c.ebp;
    r->eflags = gp4::flags::merge(r->eflags, gp4::flags::merge(c.eflags, flags));
}
// Spec: the sibling 0x0040df1c's composition plus gridlinks_008943a0. 0x004430d1 -> 0x004097c7 -> 0x004097fd
// (when [0x7c2e75] == 0) puts cars 0/1 on the pit-box record [0x866f08] + 0x160 + 0x420 * row (0x0043207c,
// row +0xfd = 0..11 from 0x0043b954), i.e. up to 35 records past [0x866f08]; grid_00408d6d draws [0x866f08]
// 54 records BEFORE record 0..11 (for 0x00408d6d's 0x0040617d start with [0x7c2ea9] bit 7 clear), which put
// those cars on wild memory below 0x8943a0 whose +0x44 link 0x00406b0a follows (faults at 0x00406306). The
// synthetic track cannot hold both, so [0x866f08] is record 0..9 (as cur_00444c9a, links via gridlinks) and
// [0x7c2ea9] keeps bit 7 set (0x00408d6d then starts from the lap start [0x891504]).
GP4_IMPL(0x0040df63, FUN_0040df63, "regs(edx:u32, ebp:u32) -> (eax, ebx, ecx, edx, esi, edi, ebp, cf, pf, af, zf, sf, of); maxinsn=80000000; wall=3600; globals=layout=setup_007adb54+trackgeo_008943a0+tbl_00868598+state_0040d8a5+state_0041104f+flag_007a97a0+cars_0066fef0+list_0066fe3c+lanes_0040da62+pend_0040daeb+state_00412af5+state_00403800+div_00403bb6+state_0040f629+ranges_004322a6+state_0040705f+frac_0040705f+state_00406b0a+ring_00406ac7+bounds_00406a57+ring_00405a12+links_00405a12+gridlinks_008943a0+pit_00406dbb+state_0040759e+trackfar_008943a0+segflag_004381ab+ctl_007ab548+ctl_007ab590+carst_0040f6d0+carf_00410d04+state_004059ef+state_004094e3+trk_007ad894+circ_007adb50+lvl_007adb86+codes_0043ba12+wheel_00410a1c+state_0066d8c0+words_0062d210+state_0042595c+carno_0066ff6c+state_0040f112+gain_00408790+state_004087aa+div_0060138c+state_0040860a+state_00405f77+levels_006f2cc4+tbl_0066d94c+state_0043c1ba+state_0043c31b+state_0043ca8b+state_0043c4d9+tabs_0043ced2+state_00411109+scale_0066ec60+keys_00408c3e+state_00431ffb+start_004098fd+pick_00443148+mode_004430d1+grid_00408d6d+order_00675958+search_0066d82c+state_004121c7+state_004120e4+state_00411f9f+state_004123a6+frac_00624790+state_00416708+state_0040dfe9+flags_00408f7b+lap_0043b482+state_0044334d+state_004320f2+neg22_00520023; gfields={0x0066d90a*22/1:u8{=0}, 0x0066fb80*22/4:u32{=0x21980000}, 0x0066fbd8*22/4:u32{=0x21980000}, 0x0066fc30*22/4:u32{=0x21980000}, 0x0066fc88*22/4:u32{=0x21980000}, 0x0066ec4a*22/1:u8{=0}, 0x0066d920*22/1:u8{=0,2,0x40,0x80,0xfe}, 0x00601d68:i32[0x100000..0x7fffffff], 0x00601d8c*4/4:i32[16..32000], 0x007c2ea9:u8{=0x80,0xff,0xc4,0x81,0xc2}, 0x00866f08:u32{=0x8943a0,0x894500,0x894660,0x8947c0,0x894920,0x894a80,0x894be0,0x894d40,0x894ea0,0x895000}}")
