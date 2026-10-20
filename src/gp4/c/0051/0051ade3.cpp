#include <gp4/gp4.h>

// Points esi at the first car record (0x0066fef0) and calls 0x00418901, which returns in eax the
// address of the 0xa8-byte entry selected by the byte at record + 0x202. Flags are those of 0x00418901.
// FUNCTION: GP4 0x0051ade3
void __cdecl FUN_0051ade3(gp4::Regs* r) {
    r->esi = 0x0066fef0u;
    gp4::call_regs(0x00418901, *r);
}
GP4_IMPL(0x0051ade3, FUN_0051ade3, "regs() -> (eax, esi, cf, pf, af, zf, sf, of); globals=layout=carbg_0066fef0")
