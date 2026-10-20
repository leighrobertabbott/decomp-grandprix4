#include <gp4/gp4.h>
#include <gp4/flags.h>

// Menu item value handler for the command in edx (esi = the item). Commands 2 and 3 run the generic
// handler 0x005394b1 and then the scroll smoothing step 0x0054328e (the registers and the flags come
// straight out of the last call); commands 5 and 6 clear the dword [0x0082fd88]; command 1 and any
// other command do nothing. edx comes back 0 on every path; for the paths that do not call, the flags
// are those of the last `cmp edx, K` executed.
// FUNCTION: GP4 0x0054330b
void __cdecl FUN_0054330b(gp4::Regs* r) {
    const uint32_t command = r->edx;
    uint32_t arith;

    if (command == 1u) {
        arith = gp4::flags::sub32(command, 1u);
    } else if (command == 2u || command == 3u) {
        gp4::call_regs(0x005394b1, *r);
        gp4::call_regs(0x0054328e, *r);
        r->edx = 0u;
        return;
    } else if (command == 5u) {
        GP4_GLOBAL(uint32_t, 0x0082fd88) = 0u;
        arith = gp4::flags::sub32(command, 5u);
    } else {
        arith = gp4::flags::sub32(command, 6u);
        if (command == 6u) {
            GP4_GLOBAL(uint32_t, 0x0082fd88) = 0u;
        }
    }

    r->edx = 0u;
    r->eflags = gp4::flags::merge(r->eflags, arith);
}
GP4_IMPL(0x0054330b, FUN_0054330b, "regs(eax:u32, ebx:u32, edx:u32{1,2,3,5,6,0,4,2,3}, esi:u32{=0x843040,0x843066,0x84308c,0x8430b2,0x8430d8,0x8430fe,0x843128,0x843152,0x84317c,0x8431a6,0x8431d0,0x8431fa,0x843224}) -> (eax, ebx, edx, cf, pf, af, zf, sf, of); globals=fuzz:i32+layout=menu_0084300a+scroll_00543340+flag4_005394b1")
