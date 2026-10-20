#include <gp4/gp4.h>

// Runs 0x0040b0da on each of the 22 car records at 0x0066fef0 (stride 0x41c); esi ends one record past the
// array and ecx at 0. The flags are those of the final `dec ecx` (result 0: ZF and PF set, CF kept from
// the preceding `add esi, 0x41c`, which does not carry).
// FUNCTION: GP4 0x0040b232
void __cdecl FUN_0040b232(gp4::Regs* r) {
    gp4::Regs call = *r;
    call.esi = 0x0066fef0u;
    call.ecx = 0x16u;
    do {
        gp4::call_regs(0x0040b0da, call);
        call.esi += 0x41cu;
        call.ecx -= 1u;
    } while (call.ecx != 0u);
    call.eflags = (call.eflags & ~0x8d5u) | 0x44u;
    *r = call;
}
GP4_IMPL(0x0040b232, FUN_0040b232, "regs() -> (ecx, esi, cf, pf, af, zf, sf, of); globals=fuzz:i32")
