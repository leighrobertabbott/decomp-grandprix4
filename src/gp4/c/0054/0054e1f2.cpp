#include <gp4/gp4.h>
#include <gp4/flags.h>

// Loads a pair of state values after the empty hook 0x0054d6f2: ebp = ebx = [0x00833cc5], then
// ebx -= eax (the incoming eax, which the hook leaves alone); eax = [0x00833cc9] and
// ecx = eax + [0x00833f58]. The flags are those of that final add.
// FUN_0054e1f2 reads eax at entry, so the register convention is needed.
// FUNCTION: GP4 0x0054e1f2
void __cdecl FUN_0054e1f2(gp4::Regs* r) {
    gp4::Regs c = *r;
    gp4::call_regs(0x0054d6f2, c);

    const uint32_t base = GP4_GLOBAL(uint32_t, 0x00833cc5);
    const uint32_t ebx = base - c.eax;
    const uint32_t eax = GP4_GLOBAL(uint32_t, 0x00833cc9);
    const uint32_t addend = GP4_GLOBAL(uint32_t, 0x00833f58);

    r->eax = eax;
    r->ebx = ebx;
    r->ecx = eax + addend;
    r->edx = c.edx;
    r->esi = c.esi;
    r->edi = c.edi;
    r->ebp = base;
    r->eflags = gp4::flags::merge(c.eflags, gp4::flags::add32(eax, addend));
}
GP4_IMPL(0x0054e1f2, FUN_0054e1f2, "regs(eax:u32) -> (eax, ebx, ecx, ebp, cf, pf, af, zf, sf, of); globals=fuzz:i32")
