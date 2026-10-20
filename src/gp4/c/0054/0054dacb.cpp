#include <gp4/gp4.h>
#include <gp4/flags.h>

// push ebx; mov bh, 0x80; call 0x0054dad5; pop ebx; ret
// Writes the signed decimal digits of eax (a '-' prefix for negative values, no padding) to the cursor
// edi in the game's digit encoding, NUL-terminated. The callee keeps every register except edi, which
// ends on the terminator; ebx is restored by the pop. The flags are those the callee leaves.
// FUNCTION: GP4 0x0054dacb
void __cdecl FUN_0054dacb(gp4::Regs* r) {
    gp4::Regs c = *r;
    c.st_in = 0;
    c.st_out = 0;
    c.ebx = (r->ebx & 0xffff00ffu) | 0x8000u;
    gp4::call_regs(0x0054dad5, c);
    r->edi = c.edi;
    r->eflags = gp4::flags::merge(r->eflags, c.eflags);
}
GP4_IMPL(0x0054dacb, FUN_0054dacb, "regs(eax:u32, edi:ptr[0x40]:bytes) -> (edi, cf, pf, af, zf, sf, of); globals=fuzz:i32")
