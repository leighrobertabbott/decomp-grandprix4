#include <gp4/gp4.h>
#include <gp4/flags.h>

// Text-edit record at edi: locates the cursor with 0x0054e045 (dh = line, dl = column). When the
// cursor is on line 0 nothing else happens (flags of `or dh, dh`); otherwise it walks the text back
// to line dh - 1 at the same column with 0x0054e020 and stores the position reached (esi) as the
// record's cursor (+8). eax, edx, esi and the flags are those of the last routine that ran.
// FUNCTION: GP4 0x0054de4b
void __cdecl FUN_0054de4b(gp4::Regs* r) {
    gp4::Regs c = *r;
    gp4::call_regs(0x0054e045, c);
    const uint32_t line = (c.edx >> 8) & 0xffu;
    uint32_t flags = gp4::flags::logic8(line);
    if (line != 0u) {
        c.edx = (c.edx & 0xffff00ffu) | (((line - 1u) & 0xffu) << 8);
        gp4::call_regs(0x0054e020, c);
        flags = c.eflags;
        GP4_FIELD(uint32_t, r->edi, 8) = c.esi;
    }
    r->eax = c.eax;
    r->edx = c.edx;
    r->esi = c.esi;
    r->eflags = gp4::flags::merge(r->eflags, flags);
}
GP4_IMPL(0x0054de4b, FUN_0054de4b, "regs(eax:u32, edx:u32, edi:ptr[0x10]:u32{=0xa20000,0xa20004,0xa20010,0xa20020,0xa20030}) -> (eax, edx, esi, cf, pf, af, zf, sf, of); globals=layout=text_0054e12a")
