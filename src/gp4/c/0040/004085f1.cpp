#include <gp4/gp4.h>

// pushal; call 0x0040bcb9; popal; ret
// The callee (a regs routine that stores two products to DAT_00622828 / DAT_0062282c) gets the
// entry registers; popal then restores every general register to its entry value but leaves the
// flags of the callee's last flag writer (its final imul).
// FUNCTION: GP4 0x004085f1
void __cdecl FUN_004085f1(gp4::Regs* r) {
    const uint32_t eax = r->eax;
    const uint32_t ebx = r->ebx;
    const uint32_t ecx = r->ecx;
    const uint32_t edx = r->edx;
    const uint32_t esi = r->esi;
    const uint32_t edi = r->edi;
    const uint32_t ebp = r->ebp;

    gp4::call_regs(0x0040bcb9, *r);

    r->eax = eax;
    r->ebx = ebx;
    r->ecx = ecx;
    r->edx = edx;
    r->esi = esi;
    r->edi = edi;
    r->ebp = ebp;
}
GP4_IMPL(0x004085f1, FUN_004085f1, "regs() -> (cf, pf, af, zf, sf, of); globals=fuzz:i32")
