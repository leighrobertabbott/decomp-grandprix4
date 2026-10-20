#include <gp4/gp4.h>

// Advances the record pointer in edi by one 0x160-byte record, then remaps it through the
// routine at 0x00406051 (which also leaves the flags of its last compare).
// FUNCTION: GP4 0x00406045
void __cdecl FUN_00406045(gp4::Regs* r) {
    r->edi += 0x160u;
    gp4::call_regs(0x00406051, *r);
}
GP4_IMPL(0x00406045, FUN_00406045, "regs(edi:u32[0xfffffea0..0xfffffea3]) -> (edi, cf, pf, af, zf, sf, of); globals=layout=remap_00406187")
