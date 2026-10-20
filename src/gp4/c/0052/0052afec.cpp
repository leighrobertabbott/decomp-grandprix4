#include <gp4/gp4.h>
#include <gp4/flags.h>

// Formats the dword [0x0081d368] as a zero-padded (bh = 0x40) 4-digit field (cl = 4, pad character
// bl = 0x16 = the game's '0') at the cursor edi through the number writer 0x0054dad5. The writer keeps
// every register but edi (it ends on the NUL); bh, bl and cl stay as set here.
// FUNCTION: GP4 0x0052afec
void __cdecl FUN_0052afec(gp4::Regs* r) {
    gp4::Regs c = *r;
    c.st_in = 0;
    c.st_out = 0;
    c.eax = GP4_GLOBAL(uint32_t, 0x0081d368);
    c.ebx = (c.ebx & 0xffff0000u) | 0x4016u;
    c.ecx = (c.ecx & 0xffffff00u) | 4u;
    gp4::call_regs(0x0054dad5, c);
    r->eax = c.eax;
    r->ebx = c.ebx;
    r->ecx = c.ecx;
    r->edi = c.edi;
    r->eflags = gp4::flags::merge(r->eflags, c.eflags);
}
GP4_IMPL(0x0052afec, FUN_0052afec, "regs(ebx:u32, ecx:u32, edi:ptr[0x40]:bytes) -> (eax, ebx, ecx, edi, cf, pf, af, zf, sf, of); globals=fuzz:i32")
