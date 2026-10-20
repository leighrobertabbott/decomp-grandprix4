#include <gp4/gp4.h>

// Runs the four update routines in order (0x0051f0c7, 0x004454a4, 0x00444ae9, 0x004453d1). Each is a register-convention
// routine, so one register block is threaded through all four calls and handed back as the wrapper's outputs.
// FUNCTION: GP4 0x0044528b
void __cdecl FUN_0044528b(gp4::Regs* r) {
    gp4::Regs regs = *r;
    regs.st_in = 0;
    regs.st_out = 0;
    gp4::call_regs(0x0051f0c7, regs);
    gp4::call_regs(0x004454a4, regs);
    gp4::call_regs(0x00444ae9, regs);
    gp4::call_regs(0x004453d1, regs);
    *r = regs;
}
GP4_IMPL(0x0044528b, FUN_0044528b, "regs() -> (eax, ebx, ecx, edx, esi, cf, pf, af, zf, sf, of); globals=fuzz:i32+layout=state_0040fa01+state_0041104f+carx_0042fe2f+state_00413f28")
