#include <gp4/gp4.h>
#include <gp4/flags.h>

// Menu item value handler for the command in edx (esi = the item, ebx = its value cell). Command 1 returns
// edx = 0 (flags of `cmp edx, 1`). Any other command runs the generic stepper 0x00539182 and then the scroll
// smoothing step 0x0054328e; registers and flags come straight out of the last call.
// FUNCTION: GP4 0x005432dd
void __cdecl FUN_005432dd(gp4::Regs* r) {
    const uint32_t command = r->edx;
    r->eflags = gp4::flags::merge(r->eflags, gp4::flags::sub32(command, 1u));
    if (command == 1u) {
        r->edx = 0u;
        return;
    }
    gp4::call_regs(0x00539182, *r);
    gp4::call_regs(0x0054328e, *r);
}
GP4_IMPL(0x005432dd, FUN_005432dd, "regs(eax:u32, edx:u32{1,2,3,0,2,3,4}, ebx:ptr[8]:u8{0,0,0,1,2}, esi:ptr[0x40]:u8{0,0,0,0,0,0,1,2,3,0x80}) -> (eax, edx, cf, pf, af, zf, sf, of); globals=fuzz:i32+layout=scroll_00543340")
