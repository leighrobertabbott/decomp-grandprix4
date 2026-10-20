#include <gp4/gp4.h>

// pushal; call 0x00433ea1; popal; ret
// The callee gets the entry registers; popal then restores every general register to its entry
// value but leaves the flags of the callee's last flag writer.
// FUNCTION: GP4 0x004084dc
void __cdecl FUN_004084dc(gp4::Regs* r) {
    const uint32_t eax = r->eax;
    const uint32_t ebx = r->ebx;
    const uint32_t ecx = r->ecx;
    const uint32_t edx = r->edx;
    const uint32_t esi = r->esi;
    const uint32_t edi = r->edi;
    const uint32_t ebp = r->ebp;

    gp4::call_regs(0x00433ea1, *r);

    r->eax = eax;
    r->ebx = ebx;
    r->ecx = ecx;
    r->edx = edx;
    r->esi = esi;
    r->edi = edi;
    r->ebp = ebp;
}
GP4_IMPL(0x004084dc, FUN_004084dc, "regs() -> (cf, pf, af, zf, sf, of); globals=fuzz:i32")
