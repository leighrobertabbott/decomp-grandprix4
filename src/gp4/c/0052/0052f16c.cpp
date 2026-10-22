#include <gp4/gp4.h>
#include <gp4/flags.h>

// Draws the text with id 0x4bb (0x0054d1a2; esi is saved around it), then prints the two bytes at
// the caller's string esi as "<number>(" ... ")" style pairs: each byte is formatted by the decimal
// number writer 0x0054d969 (bh cleared, so no sign/width mode), followed by one character through
// 0x0054d3a1 ('(' = 0x28 after the first, ')' = 0x29 after the second). Finally esi skips two more
// bytes and the text cursor line advance 0x0052f1f0 runs. Every callee preserves the registers it
// does not own, so one Regs is threaded through; the flags are the last callee's.
// FUNCTION: GP4 0x0052f16c
void __cdecl FUN_0052f16c(gp4::Regs* r) {
    const uint8_t* text = reinterpret_cast<const uint8_t*>(r->esi);

    gp4::Regs c = *r;
    c.st_in = 0;
    c.st_out = 0;
    c.st_xin_mask = 0;
    c.st_xout_mask = 0;
    c.esi = 0x000004bbu;
    gp4::call_regs(0x0054d1a2, c);

    c.esi = r->esi;
    c.eax = text[0];
    c.ebx &= ~0xff00u;
    gp4::call_regs(0x0054d969, c);
    c.eax = (c.eax & ~0xffu) | 0x28u;
    gp4::call_regs(0x0054d3a1, c);

    c.eax = text[1];
    c.ebx &= ~0xff00u;
    gp4::call_regs(0x0054d969, c);
    c.eax = (c.eax & ~0xffu) | 0x29u;
    gp4::call_regs(0x0054d3a1, c);

    c.esi = r->esi + 4u;
    gp4::call_regs(0x0052f1f0, c);

    r->eax = c.eax;
    r->ebx = c.ebx;
    r->ecx = c.ecx;
    r->edx = c.edx;
    r->esi = c.esi;
    r->edi = c.edi;
    r->ebp = c.ebp;
    r->eflags = gp4::flags::merge(r->eflags, c.eflags);
}
GP4_IMPL(0x0052f16c, FUN_0052f16c, "regs(ebx:u32, ecx:u32[0..14], esi:ptr[0x40]:bytes) -> (eax, ebx, esi, cf, pf, af, zf, sf, of); globals=fuzz:i32+layout=log_004fd730+script_all_00835846+textout_0054d211+lim_0052f1f0")
