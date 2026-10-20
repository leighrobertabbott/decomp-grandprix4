#include <gp4/gp4.h>

// Stores the record pointer in edi at 0x00602d84 and 0x00602d88, then runs the two pushal-wrapped
// routines 0x004057cf and 0x004279ab. Every register is restored by their popal; the flags are those
// 0x004279ab leaves.
// FUNCTION: GP4 0x0040587c
void __cdecl FUN_0040587c(gp4::Regs* r) {
    GP4_GLOBAL(uint32_t, 0x00602d84) = r->edi;
    GP4_GLOBAL(uint32_t, 0x00602d88) = r->edi;
    gp4::Regs call = *r;
    gp4::call_regs(0x004057cf, call);
    gp4::call_regs(0x004279ab, call);
    *r = call;
}
GP4_IMPL(0x0040587c, FUN_0040587c, "regs(esi:ptr[0x28c]:bytes, edi:rec[0x8943a0,0x160,0..25]) -> (cf, pf, af, zf, sf, of); globals=fuzz:i32")
