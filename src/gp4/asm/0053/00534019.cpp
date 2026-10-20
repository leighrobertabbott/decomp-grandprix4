#include <gp4/gp4.h>
#include <gp4/flags.h>

// Menu value handler: the command arrives in edx. Command 1 clears bit 7 of the item flag byte (+0x1f);
// when bit 7 of +0x1e is clear it compares the item's value byte (+0xc, less one) with the cell at ebx
// through 0x005398c4 and sets the flag bit again on equality; edx is cleared afterwards. Command 5
// copies the dword at 0x00828297 to 0x00828293 and refills the table through 0x0053414a. Any other
// command only compares. The flags are those of the last flag writer on each path.
// FUNCTION: GP4 0x00534019
void __cdecl FUN_00534019(gp4::Regs* r) {
    const uint32_t cmd = r->edx;
    const uint32_t item = r->esi;
    uint32_t arith = gp4::flags::sub32(cmd, 5u);

    if (cmd == 1u) {
        GP4_FIELD(uint8_t, item, 0x1f) = static_cast<uint8_t>(GP4_FIELD(uint8_t, item, 0x1f) & 0x7fu);
        const uint8_t state = GP4_FIELD(uint8_t, item, 0x1e);
        arith = gp4::flags::logic8(state & 0x80u);
        if ((state & 0x80u) == 0u) {
            gp4::Regs c = *r;
            c.st_in = 0;
            c.st_out = 0;
            c.st_xin_mask = 0;
            c.st_xout_mask = 0;
            c.eax = static_cast<uint32_t>(GP4_FIELD(uint8_t, item, 0xc)) - 1u;
            gp4::call_regs(0x005398c4, c);
            r->eax = c.eax;
            arith = c.eflags;
            if (c.eflags & gp4::flags::ZF) {
                const uint8_t v = static_cast<uint8_t>(GP4_FIELD(uint8_t, item, 0x1f) | 0x80u);
                GP4_FIELD(uint8_t, item, 0x1f) = v;
                arith = gp4::flags::logic8(v);
            }
        }
        r->edx = 0u;
    } else if (cmd == 5u) {
        r->eax = GP4_GLOBAL(uint32_t, 0x00828297);
        GP4_GLOBAL(uint32_t, 0x00828293) = r->eax;
        gp4::Regs c = *r;
        c.st_in = 0;
        c.st_out = 0;
        c.st_xin_mask = 0;
        c.st_xout_mask = 0;
        gp4::call_regs(0x0053414a, c);
        r->ecx = c.ecx;
        r->edi = c.edi;
        arith = c.eflags;
    }

    r->eflags = gp4::flags::merge(r->eflags, arith);
}
GP4_IMPL(0x00534019, FUN_00534019, "regs(eax:u32, edx:u32{1,5,1,0,2}, esi:ptr[0x40]:u8{0,0,0,1,2,0x80}, ebx:ptr[8]:u8{0,0,0,1,2}) -> (eax, ecx, edx, edi, cf, pf, af, zf, sf, of); globals=fuzz:i32+layout=tab_00835846")
