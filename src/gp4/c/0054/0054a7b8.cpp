#include <gp4/gp4.h>
#include <gp4/flags.h>

// Advances the byte counter 0x0082fd6c by the byte 0x0082fd6d, runs 0x0054eb70 (its al comes back
// unchanged, the sum) and stores that byte at 0x007a97b2, then latches code 0x11 with 0x0054ac8b.
// Registers and flags come back as 0x0054ac8b leaves them.
// FUNCTION: GP4 0x0054a7b8
void __cdecl FUN_0054a7b8(gp4::Regs* r) {
    const uint32_t a = GP4_GLOBAL(uint8_t, 0x0082fd6c);
    const uint32_t b = GP4_GLOBAL(uint8_t, 0x0082fd6d);
    const uint32_t sum = (a + b) & 0xffu;
    GP4_GLOBAL(uint8_t, 0x0082fd6c) = static_cast<uint8_t>(sum);

    gp4::Regs c = *r;
    c.st_in = 0;
    c.st_out = 0;
    c.eax = (r->eax & 0xffffff00u) | sum;
    c.eflags = gp4::flags::merge(r->eflags, gp4::flags::add8(a, b));
    gp4::call_regs(0x0054eb70, c);
    GP4_GLOBAL(uint8_t, 0x007a97b2) = static_cast<uint8_t>(c.eax);

    c.eax = (c.eax & 0xffffff00u) | 0x11u;
    gp4::call_regs(0x0054ac8b, c);
    *r = c;
}
GP4_IMPL(0x0054a7b8, FUN_0054a7b8, "regs(eax:u32, edx:u32) -> (eax, ebx, ecx, edx, esi, edi, ebp, cf, pf, af, zf, sf, of); globals=fuzz:i32+layout=list_0054eaf7+links_0054eaf7+tail_0054eb10+state_0054b978+mode_007aafd3")
