#include <gp4/gp4.h>

// Posts window message 0x404 (wparam = lparam = 0) to the window handle [0x00b1c7f0] through the
// stdcall wrapper 0x0046ae90. The registers and flags come back as that callee leaves them (eax is its
// result).
// FUNCTION: GP4 0x0052cbb3
void __cdecl FUN_0052cbb3(gp4::Regs* r) {
    gp4::Regs c = *r;
    c.st_in = 0;
    c.st_out = 0;
    const uint32_t args[4] = {GP4_GLOBAL(uint32_t, 0x00b1c7f0), 0x404u, 0u, 0u};
    gp4::call_regs_stdcall(0x0046ae90, c, args);
    *r = c;
}
GP4_IMPL(0x0052cbb3, FUN_0052cbb3, "regs() -> (eax, ebx, ecx, edx, esi, edi, ebp, cf, pf, af, zf, sf, of); globals=fuzz:i32")
