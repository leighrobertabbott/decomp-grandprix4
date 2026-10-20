#include <gp4/gp4.h>
#include <gp4/flags.h>

// Menu item value handler selected by the command in edx (esi = the item). Commands 2 and 3 (and 4 when
// bit 2 of the item state byte +0x1e is set) clear the byte flag at 0x007ab003 (0x0053daa6) and store the
// dword [0x00abc080] into the cell the item points at (+0xe, returned in ebx) through 0x00539a13; when the
// selection byte [0x0081d1eb] is 0xff the same value is also stored into the second cell (+0x36, returned
// in ebx). Commands 5 and 6 clear the dword [0x0082fd88]. edx comes back 0 for every command; the flags
// are those of the last writer on the path.
// FUNCTION: GP4 0x00529ee2
void __cdecl FUN_00529ee2(gp4::Regs* r) {
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
        r->eax = c.eax;
        r->ebx = c.ebx;
        arith = gp4::flags::sub8(GP4_GLOBAL(uint8_t, 0x0081d1eb), 0xffu);
        if (GP4_GLOBAL(uint8_t, 0x0081d1eb) == 0xffu) {
            c.ebx = GP4_FIELD(uint32_t, item, 0x36);
            gp4::call_regs(0x00539a13, c);
            r->ebx = c.ebx;
            arith = c.eflags;
        }
    }

    r->edx = 0u;
    r->eflags = gp4::flags::merge(r->eflags, arith);
}
GP4_IMPL(0x00529ee2, FUN_00529ee2, "regs(eax:u32, ebx:u32, edx:u32{1,2,3,4,5,6,0,4,2,3}, esi:ptr[0x40]:layout=rec_00529ee2) -> (eax, ebx, edx, cf, pf, af, zf, sf, of); globals=fuzz:i32+layout=menu_0084300a+rec_00529ee2")
