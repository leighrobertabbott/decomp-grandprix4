#include <gp4/gp4.h>

// FUNCTION: GP4 0x00422833
void __cdecl FUN_00422833(gp4::Regs* r) {
    GP4_ARRAY(uint8_t, 0x0063bcfc)[r->eax] = 0u;
}
GP4_IMPL(0x00422833, FUN_00422833, "regs(eax:u32[0..63]) -> (); globals=layout=bytes_0063bcfc")
