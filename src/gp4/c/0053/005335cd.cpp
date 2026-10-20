#include <gp4/gp4.h>

// Seals the outgoing packet buffer with 0x0054161c (checksum appended to 0x007a97b0.., length + 4) and
// then sends it with 0x00507410(0, 0) (stdcall, two arguments). The sender preserves ebx, esi, edi and
// ebp; registers and flags otherwise come back as the callee leaves them.
// FUNCTION: GP4 0x005335cd
void __cdecl FUN_005335cd(gp4::Regs* r) {
    gp4::Regs c = *r;
    c.st_in = 0;
    c.st_out = 0;
    gp4::call_regs(0x0054161c, c);
    const uint32_t args[2] = {0u, 0u};
    gp4::call_regs_stdcall(0x00507410, c, args);
    *r = c;
}
GP4_IMPL(0x005335cd, FUN_005335cd, "regs() -> (eax, ebx, ecx, edx, esi, edi, ebp, cf, pf, af, zf, sf, of); globals=fuzz:i32+layout=state_0054b978")
