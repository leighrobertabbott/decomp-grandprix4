#include <gp4/gp4.h>

// Writes the 16-bit value in ax as four hex digits through 0x0051f3ef (two digits per byte): first the
// high byte (`mov al, ah` replaces the low byte), then the original eax (low byte). eax is restored by
// the pops; the other registers (edi, the output cursor) and the flags come from the callee.
// (v2: callee-chain state from readout_00515187)
// FUNCTION: GP4 0x0051f401
void __cdecl FUN_0051f401(gp4::Regs* r) {
    const uint32_t value = r->eax;
    gp4::Regs c = *r;
    c.eax = (value & 0xffffff00u) | ((value >> 8) & 0xffu);
    gp4::call_regs(0x0051f3ef, c);
    c.eax = value;
    gp4::call_regs(0x0051f3ef, c);
    *r = c;
}
GP4_IMPL(0x0051f401, FUN_0051f401, "regs(eax:u32, edi:ptr[0x40]:bytes) -> (eax, ebx, ecx, edx, esi, edi, ebp, cf, pf, af, zf, sf, of); globals=fuzz:i32+layout=readout_00515187")
