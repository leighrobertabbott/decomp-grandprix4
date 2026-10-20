#include <gp4/gp4.h>
#include <gp4/flags.h>

// Loads the 4-word record at esi (0x00539f40: ebx/eax/ebp/ecx = sign-extended words, esi += 8), sets ecx = ebp,
// then makes two rounds that store a 16-bit value through 0x00531af4 (first 0x1084, then 0x318c) and run the
// empty routine 0x00531b28; eax (the loaded word 1) is kept around the first round and one higher around the
// second. Exit: eax = word 1 + 1, ebx, ebp, ecx = ebp, esi + 8; the flags are those of the last store routine.
// FUNCTION: GP4 0x00539f15
void __cdecl FUN_00539f15(gp4::Regs* r) {
    gp4::Regs c = *r;
    c.st_in = 0;
    c.st_out = 0;
    c.st_xin_mask = 0;
    c.st_xout_mask = 0;
    gp4::call_regs(0x00539f40, c);
    c.ecx = c.ebp;

    const uint32_t word1 = c.eax;
    c.eax = 0x1084u;
    gp4::call_regs(0x00531af4, c);
    c.eax = word1;
    gp4::cdecl_<0x00531b28, void>();

    const uint32_t next = word1 + 1u;
    c.eax = 0x318cu;
    gp4::call_regs(0x00531af4, c);
    c.eax = next;
    gp4::cdecl_<0x00531b28, void>();

    r->eax = c.eax;
    r->ebx = c.ebx;
    r->ecx = c.ecx;
    r->ebp = c.ebp;
    r->esi = c.esi;
    r->eflags = gp4::flags::merge(r->eflags, c.eflags);
}
GP4_IMPL(0x00539f15, FUN_00539f15, "regs(esi:ptr[0x40]:bytes) -> (eax, ebx, ecx, ebp, esi, cf, pf, af, zf, sf, of); globals=fuzz:i32")
