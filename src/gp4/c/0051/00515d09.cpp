#include <gp4/gp4.h>

// Calls the car/teammate check at 0x00411017 (esi = car record) and stores its result (eax) in
// DAT_008660cc. Registers and flags are whatever 0x00411017 left.
// FUNCTION: GP4 0x00515d09
void __cdecl FUN_00515d09(gp4::Regs* r) {
    gp4::call_regs(0x00411017, *r);
    GP4_GLOBAL(uint32_t, 0x008660cc) = r->eax;
}
GP4_IMPL(0x00515d09, FUN_00515d09, "regs(esi:ptr[0x106]:layout=rec_00411017) -> (eax, cf, pf, af, zf, sf, of); globals=layout=state_00411017")
