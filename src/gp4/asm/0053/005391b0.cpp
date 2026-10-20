#include <gp4/gp4.h>
#include <gp4/flags.h>

// Menu item value handler selected by the command in edx (esi = the item, ebx = its value cell). Command 1
// returns edx = 0. Commands 2 and 3 read the cell (0x00539944), add the step in dword +0x2a of the item with
// signed compares, never going above the signed limit in dword +0x26 (a signed overflow of the addition also
// gives the limit), and write it back (0x00539a13); the flags are then those of the write. Any other command
// does nothing (flags of `cmp edx, 3`).
// FUNCTION: GP4 0x005391b0
void __cdecl FUN_005391b0(gp4::Regs* r) {
    const uint32_t command = r->edx;

    if (command == 1u) {
        r->edx = 0u;
        r->eflags = gp4::flags::merge(r->eflags, gp4::flags::sub32(command, 1u));
        return;
    }
    if (command != 2u && command != 3u) {
        r->eflags = gp4::flags::merge(r->eflags, gp4::flags::sub32(command, 3u));
        return;
    }

    gp4::Regs c = *r;
    c.st_in = 0;
    c.st_out = 0;
    c.st_xin_mask = 0;
    c.st_xout_mask = 0;
    gp4::call_regs(0x00539944, c);

    const uint32_t limit = GP4_FIELD(uint32_t, c.esi, 0x26);
    const uint32_t step = GP4_FIELD(uint32_t, c.esi, 0x2a);
    const uint32_t value = c.eax + step;
    const bool overflow = (gp4::flags::add32(c.eax, step) & gp4::flags::OF) != 0u;
    c.eax = (overflow || static_cast<int32_t>(value) > static_cast<int32_t>(limit)) ? limit : value;
    gp4::call_regs(0x00539a13, c);

    r->eax = c.eax;
    r->eflags = gp4::flags::merge(r->eflags, c.eflags);
}
GP4_IMPL(0x005391b0, FUN_005391b0, "regs(eax:u32, edx:u32{1,2,3,0,2,3,4}, ebx:ptr[8]:u8{0,0,0,1,2}, esi:ptr[0x40]:u8{0,0,0,0,0,0,1,2,3,0x80}) -> (eax, edx, cf, pf, af, zf, sf, of); globals=fuzz:i32")
