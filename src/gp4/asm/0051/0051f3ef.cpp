#include <gp4/gp4.h>

// Writes the byte in al as two hex digits through 0x0051f3db: first the high nibble (`shr al, 4`), then
// the original eax (0x0051f3db masks the low nibble). eax is restored by the pops; the other registers
// (edi, the output cursor) and the flags come from the callee.
// FUNCTION: GP4 0x0051f3ef
void __cdecl FUN_0051f3ef(gp4::Regs* r) {
    const uint32_t value = r->eax;
    gp4::Regs c = *r;
    c.eax = (value & 0xffffff00u) | ((value & 0xffu) >> 4);
    gp4::call_regs(0x0051f3db, c);
    c.eax = value;
    gp4::call_regs(0x0051f3db, c);
    *r = c;
}
GP4_IMPL(0x0051f3ef, FUN_0051f3ef, "regs(eax:u32, edi:ptr[0x40]:bytes) -> (eax, ebx, ecx, edx, esi, edi, ebp, cf, pf, af, zf, sf, of); globals=fuzz:i32+layout=readout_00515187")
