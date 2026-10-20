#include <gp4/gp4.h>
#include <gp4/flags.h>

// Menu item value handler selected by the command in edx (esi = the item, ebx = its value cell). Command 1
// returns edx = 0. Commands 2 and 3 read the cell (0x00539944), decrement it, wrap to the dword +0x2a of the
// item when the signed result is below the signed dword +0x26, write it back (0x00539a13) and set bit 0x40
// of the byte +0x1f of the item (the changed flag); the flags are then those of that `or`. Any other command
// does nothing (flags of `cmp edx, 3`).
// FUNCTION: GP4 0x005392c4
void __cdecl FUN_005392c4(gp4::Regs* r) {
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

    const uint32_t first = GP4_FIELD(uint32_t, c.esi, 0x26);
    const uint32_t last = GP4_FIELD(uint32_t, c.esi, 0x2a);
    const uint32_t value = c.eax - 1u;
    c.eax = (static_cast<int32_t>(value) < static_cast<int32_t>(first)) ? last : value;
    gp4::call_regs(0x00539a13, c);

    const uint32_t flagbyte = GP4_FIELD(uint8_t, c.esi, 0x1f) | 0x40u;
    GP4_FIELD(uint8_t, c.esi, 0x1f) = static_cast<uint8_t>(flagbyte);

    r->eax = c.eax;
    r->eflags = gp4::flags::merge(r->eflags, gp4::flags::logic8(flagbyte));
}
GP4_IMPL(0x005392c4, FUN_005392c4, "regs(eax:u32, edx:u32{1,2,3,0,2,3,4}, ebx:ptr[8]:u8{0,0,0,1,2}, esi:ptr[0x40]:u8{0,0,0,0,0,0,1,2,3,0x80}) -> (eax, edx, cf, pf, af, zf, sf, of); globals=fuzz:i32")
