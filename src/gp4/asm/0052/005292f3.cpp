#include <gp4/gp4.h>
#include <gp4/flags.h>

// Menu item handler selected by the command in edx (esi = the item). Commands 2 and 3 run the item update
// 0x005394b1 (which stores [0x00abc080] into the cell the item points at) and refresh the menu pages (0x00529408,
// 0x0052a545, 0x005413e9); commands 5 and 6 clear the dword [0x0082fd88]; every other command (including 1 and
// 4) does nothing. edx comes back 0; the registers and flags are what the last callee on the path leaves (the
// flags of `cmp edx, k` on the paths that call nothing).
// FUNCTION: GP4 0x005292f3
void __cdecl FUN_005292f3(gp4::Regs* r) {
    const uint32_t command = r->edx;

    if (command == 2u || command == 3u) {
        gp4::Regs c = *r;
        gp4::call_regs(0x005394b1, c);
        gp4::call_regs(0x00529408, c);
        gp4::call_regs(0x0052a545, c);
        gp4::call_regs(0x005413e9, c);
        c.edx = 0u;
        *r = c;
        return;
    }

    uint32_t arith;
    if (command == 1u) {
        arith = gp4::flags::sub32(command, 1u);
    } else if (command == 5u) {
        arith = gp4::flags::sub32(command, 5u);
        GP4_GLOBAL(uint32_t, 0x0082fd88) = 0u;
    } else {
        arith = gp4::flags::sub32(command, 6u);
        if (command == 6u) {
            GP4_GLOBAL(uint32_t, 0x0082fd88) = 0u;
        }
    }
    r->edx = 0u;
    r->eflags = gp4::flags::merge(r->eflags, arith);
}
GP4_IMPL(0x005292f3, FUN_005292f3, "regs(eax:u32, ebx:u32, edx:u32{1,2,3,2,3,0,4,5,6}, esi:u32{=0x843040,0x843066,0x84308c,0x8430b2,0x8430d8,0x8430fe,0x843128,0x843152,0x84317c,0x8431a6,0x8431d0,0x8431fa,0x843224}) -> (eax, ebx, edx, cf, pf, af, zf, sf, of); globals=fuzz:i32+layout=session_004fd1a0+trk_007ad894+ramp_0081d1f2+menu_00529408+menu_0084300a+flag4_005394b1")
