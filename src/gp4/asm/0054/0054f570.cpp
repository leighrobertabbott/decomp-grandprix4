#include <gp4/gp4.h>
#include <gp4/flags.h>

// Menu item value handler selected by the command in edx (esi = the item). Commands 2 and 3 (and 4 when bit 2
// of the item state byte +0x1e is set) clear the byte flag at 0x007ab003 (0x0053daa6), store the dword
// [0x00abc080] into the cell the item points at (+0xe, returned in ebx) through 0x00539a13 and run the screen
// refresh 0x0054f54b. Commands 5 and 6 clear the dword [0x0082fd88]; command 1 and any other do nothing. edx
// comes back 0 for every command; the flags are those of the last writer on the path taken.
// FUNCTION: GP4 0x0054f570
void __cdecl FUN_0054f570(gp4::Regs* r) {
    const uint32_t command = r->edx;
    const uint32_t item = r->esi;
    uint32_t arith;
    bool enter = false;

    if (command == 1u) {
        arith = gp4::flags::sub32(command, 1u);
    } else if (command == 2u) {
        arith = gp4::flags::sub32(command, 2u);
        enter = true;
    } else if (command == 3u) {
        arith = gp4::flags::sub32(command, 3u);
        enter = true;
    } else if (command == 4u) {
        const uint8_t state = GP4_FIELD(uint8_t, item, 0x1e);
        arith = gp4::flags::logic8(state & 4u);
        enter = (state & 4u) != 0u;
    } else if (command == 5u) {
        GP4_GLOBAL(uint32_t, 0x0082fd88) = 0u;
        arith = gp4::flags::sub32(command, 5u);
    } else {
        arith = gp4::flags::sub32(command, 6u);
        if (command == 6u) {
            GP4_GLOBAL(uint32_t, 0x0082fd88) = 0u;
        }
    }

    if (enter) {
        gp4::cdecl_<0x0053daa6, void>();
        gp4::Regs c = *r;
        c.st_in = 0;
        c.st_out = 0;
        c.st_xin_mask = 0;
        c.st_xout_mask = 0;
        c.eax = GP4_GLOBAL(uint32_t, 0x00abc080);
        c.ebx = GP4_FIELD(uint32_t, item, 0xe);
        gp4::call_regs(0x00539a13, c);
        gp4::call_regs(0x0054f54b, c);
        r->eax = c.eax;
        r->ebx = c.ebx;
        arith = c.eflags;
    }

    r->edx = 0u;
    r->eflags = gp4::flags::merge(r->eflags, arith);
}
GP4_IMPL(0x0054f570, FUN_0054f570, "regs(eax:u32, ebx:u32, edx:u32{1,2,3,4,5,6,0,4,2,3}, esi:u32{=0x843040,0x843066,0x84308c,0x8430b2,0x8430d8,0x8430fe,0x843128,0x843152,0x84317c,0x8431a6,0x8431d0,0x8431fa,0x843224}) -> (eax, ebx, edx, cf, pf, af, zf, sf, of); globals=fuzz:i32+layout=menu_0084300a+flag4_005394b1")
