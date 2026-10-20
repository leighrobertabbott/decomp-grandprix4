#include <gp4/gp4.h>
#include <gp4/flags.h>

// Waits for the 'z' key: repeats the key wait 0x0052d70d (it pumps messages and pops key-queue words
// until one has a nonzero high byte) until the character in al is 0x7a. eax and the flags are those of
// the last call and its `cmp al, 0x7a`.
// FUNCTION: GP4 0x0052d764
void __cdecl FUN_0052d764(gp4::Regs* r) {
    gp4::Regs c = *r;
    c.st_in = 0;
    c.st_out = 0;
    do {
        gp4::call_regs(0x0052d70d, c);
    } while ((c.eax & 0xffu) != 0x7au);
    r->eax = c.eax;
    r->eflags = gp4::flags::merge(r->eflags, gp4::flags::sub8(c.eax & 0xffu, 0x7au));
}
GP4_IMPL(0x0052d764, FUN_0052d764, "regs(eax:u32) -> (eax, cf, pf, af, zf, sf, of); globals=fuzz:i32+layout=keyq_008245c0; emu=skip:win32 PeekMessageA")
