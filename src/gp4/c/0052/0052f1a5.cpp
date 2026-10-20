#include <gp4/gp4.h>
#include <gp4/flags.h>

// When the byte [0x826e3e] is 0x80: draws the text with id 0x4b9 (0x0054d1a2; esi is saved around it),
// then runs the one-character-at-a-time string drawer 0x0052f1c5 on the caller's string esi with at
// most ecx = 0x12 characters. Otherwise only the flags of `cmp byte [0x826e3e], 0x80` change.
// FUNCTION: GP4 0x0052f1a5
void __cdecl FUN_0052f1a5(gp4::Regs* r) {
    const uint8_t mode = GP4_GLOBAL(uint8_t, 0x00826e3e);
    if (mode != 0x80u) {
        r->eflags = gp4::flags::merge(r->eflags, gp4::flags::sub8(mode, 0x80u));
        return;
    }

    gp4::Regs c = *r;
    c.st_in = 0;
    c.st_out = 0;
    c.st_xin_mask = 0;
    c.st_xout_mask = 0;
    c.esi = 0x000004b9u;
    gp4::call_regs(0x0054d1a2, c);
    c.esi = r->esi;
    c.ecx = 0x12u;
    gp4::call_regs(0x0052f1c5, c);

    r->eax = c.eax;
    r->ebx = c.ebx;
    r->ecx = c.ecx;
    r->edx = c.edx;
    r->esi = c.esi;
    r->edi = c.edi;
    r->ebp = c.ebp;
    r->eflags = gp4::flags::merge(r->eflags, c.eflags);
}
GP4_IMPL(0x0052f1a5, FUN_0052f1a5, "regs(eax:u32, ecx:u32, esi:ptr[0x40]:u8{0,1,2,0x20,0x41,0x7f,0x80,0xff}) -> (eax, ebx, ecx, esi, cf, pf, af, zf, sf, of); globals=fuzz:i32+layout=log_004fd730+textout_0054d211")
