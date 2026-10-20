#include <gp4/gp4.h>
#include <gp4/flags.h>

// `push edx ; call 0x00538f91 ; pop edx ; cmp edx, 2 ; jne ; call 0x00530022 ; mov edx, 0 ; ret`:
// runs the menu value handler, then for command 2 also rebinds the static menu items to the
// render-state record (0x00530022). edx is cleared; the flags are those of `cmp edx, 2` or of the
// last flag writer inside 0x00530022. Twin of 0x005327f7 (which calls 0x005327ec instead).
// FUNCTION: GP4 0x00530253
void __cdecl FUN_00530253(gp4::Regs* r) {
    const uint32_t command = r->edx;
    gp4::call_regs(0x00538f91, *r);
    r->edx = command;
    r->eflags = gp4::flags::merge(r->eflags, gp4::flags::sub32(command, 2u));
    if (command == 2u) {
        gp4::call_regs(0x00530022, *r);
    }
    r->edx = 0u;
}
GP4_IMPL(0x00530253, FUN_00530253, "regs(eax:u32, edx:u32{1,2,5,6,0,1,2}, ebx:ptr[8]:u8{0,0,0,1,2}, esi:ptr[0x40]:u8{0,0,0,1,1,2,3,0x80}) -> (eax, edx, cf, pf, af, zf, sf, of); globals=fuzz:i32+layout=sel_00531859+rsrec_007ad638+flag_008270d7")
