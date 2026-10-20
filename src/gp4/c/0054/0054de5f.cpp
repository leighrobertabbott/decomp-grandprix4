#include <gp4/gp4.h>
#include <gp4/flags.h>

// Text-edit record at edi: locates the cursor with 0x0054e045 (dh = line, dl = column of the position
// reached), moves one line further (dh + 1) and walks the text back to that line and column with
// 0x0054e020; the position reached (esi) becomes the record's cursor (+8). eax, edx, esi and the
// flags are those the second walk leaves.
// FUNCTION: GP4 0x0054de5f
void __cdecl FUN_0054de5f(gp4::Regs* r) {
    gp4::Regs c = *r;
    gp4::call_regs(0x0054e045, c);
    c.edx = (c.edx & 0xffff00ffu) | ((((c.edx >> 8) + 1u) & 0xffu) << 8);
    gp4::call_regs(0x0054e020, c);
    GP4_FIELD(uint32_t, r->edi, 8) = c.esi;
    r->eax = c.eax;
    r->edx = c.edx;
    r->esi = c.esi;
    r->eflags = gp4::flags::merge(r->eflags, c.eflags);
}
GP4_IMPL(0x0054de5f, FUN_0054de5f, "regs(eax:u32, edx:u32, edi:ptr[0x10]:u32{=0xa20000,0xa20004,0xa20010,0xa20020,0xa20030}) -> (eax, edx, esi, cf, pf, af, zf, sf, of); globals=layout=text_0054e12a")
