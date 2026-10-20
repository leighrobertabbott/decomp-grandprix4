#include <gp4/gp4.h>

// pushal; call 0x0040fa3b; popal; ret
// The callee (a regs routine that clears the byte flag at 0x0095404e once it ran 0x0040fa51)
// gets the entry registers; popal then restores every general register to its entry value but
// leaves the flags of the callee's last flag writer.
// FUNCTION: GP4 0x0040fa33
void __cdecl FUN_0040fa33(gp4::Regs* r) {
    const uint32_t eax = r->eax;
    const uint32_t ebx = r->ebx;
    const uint32_t ecx = r->ecx;
    const uint32_t edx = r->edx;
    const uint32_t esi = r->esi;
    const uint32_t edi = r->edi;
    const uint32_t ebp = r->ebp;

    gp4::call_regs(0x0040fa3b, *r);

    r->eax = eax;
    r->ebx = ebx;
    r->ecx = ecx;
    r->edx = edx;
    r->esi = esi;
    r->edi = edi;
    r->ebp = ebp;
}
GP4_IMPL(0x0040fa33, FUN_0040fa33, "regs() -> (cf, pf, af, zf, sf, of); globals=fuzz:i32")
