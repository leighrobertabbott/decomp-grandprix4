#include <gp4/gp4.h>

// Writes the 32-bit value in eax as eight hex digits through 0x0051f401: first the high word
// (`shr eax, 16`), then the original eax (whose low word 0x0051f401 prints). eax is restored by the
// pops; the other registers (edi, the output cursor) and the flags come from the callee.
// FUNCTION: GP4 0x0051f412
void __cdecl FUN_0051f412(gp4::Regs* r) {
    const uint32_t value = r->eax;
    gp4::Regs c = *r;
    c.eax = value >> 16;
    gp4::call_regs(0x0051f401, c);
    c.eax = value;
    gp4::call_regs(0x0051f401, c);
    *r = c;
}
GP4_IMPL(0x0051f412, FUN_0051f412, "regs(eax:u32, edi:ptr[0x40]:bytes) -> (eax, ebx, ecx, edx, esi, edi, ebp, cf, pf, af, zf, sf, of); globals=fuzz:i32+layout=readout_00515187")
