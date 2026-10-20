#include <gp4/gp4.h>
#include <gp4/flags.h>

// Menu item handler selected by the command in edx (esi = the item). Command 2 (activation) runs 0x0052a1a0,
// the item activation 0x0052d5f8, 0x0052a12d, refreshes the menu pages (0x00529408, 0x0052a545, 0x005413e9) and
// sets the byte [0x0081e4fb] = 0xff; commands 5 and 6 clear the dword [0x0082fd88]; every other command
// (including 1 and 3) does nothing. edx comes back 0; the registers and flags are what the last callee on the
// path leaves (the flags of `cmp edx, k` on the paths that call nothing).
// FUNCTION: GP4 0x005290ac
void __cdecl FUN_005290ac(gp4::Regs* r) {
    const uint32_t command = r->edx;

    if (command == 2u) {
        gp4::Regs c = *r;
        gp4::call_regs(0x0052a1a0, c);
        gp4::call_regs(0x0052d5f8, c);
        gp4::call_regs(0x0052a12d, c);
        gp4::call_regs(0x00529408, c);
        gp4::call_regs(0x0052a545, c);
        gp4::call_regs(0x005413e9, c);
        GP4_GLOBAL(uint8_t, 0x0081e4fb) = 0xffu;
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
GP4_IMPL(0x005290ac, FUN_005290ac, "regs(eax:u32, edx:u32{1,2,2,2,0,5,6,3}, esi:ptr[0x40]:layout=mitem1_0052d5f8) -> (eax, ebx, ecx, edx, esi, edi, ebp, cf, pf, af, zf, sf, of); globals=fuzz:i32+layout=session_004fd1a0+trk_007ad894+ramp_0081d1f2+menu_00529408+ent_0052a0ec+menu_0052d5f8")
