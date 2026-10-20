#include <gp4/gp4.h>
#include <gp4/flags.h>

// Menu item value handler for the command in edx (esi = the item, ebx = its value cell). Command 1 returns
// edx = 0 (flags of `cmp edx, 1`). Any other command runs the generic stepper 0x005391b0 and then the scroll
// smoothing step 0x00543340; registers and flags come straight out of the last call.
// FUNCTION: GP4 0x005433a6
void __cdecl FUN_005433a6(gp4::Regs* r) {
    const uint32_t command = r->edx;
    r->eflags = gp4::flags::merge(r->eflags, gp4::flags::sub32(command, 1u));
    if (command == 1u) {
        r->edx = 0u;
        return;
    }
    gp4::call_regs(0x005391b0, *r);
    gp4::call_regs(0x00543340, *r);
}
GP4_IMPL(0x005433a6, FUN_005433a6, "regs(eax:u32, edx:u32{1,2,3,0,2,3,4}, ebx:ptr[8]:u8{0,0,0,1,2}, esi:ptr[0x40]:u8{0,0,0,0,0,0,1,2,3,0x80}) -> (eax, edx, cf, pf, af, zf, sf, of); globals=fuzz:i32+layout=scroll_00543340")
