#include <gp4/gp4.h>

// Stores ebx at the (unaligned) dword 0x00833cc5 (`mov [0x833cc5], ebx; ret`).
// FUNCTION: GP4 0x0054cbca
void __cdecl FUN_0054cbca(gp4::Regs* r) {
    GP4_GLOBAL(uint32_t, 0x00833cc5) = r->ebx;
}
GP4_IMPL(0x0054cbca, FUN_0054cbca, "regs(ebx:u32) -> ()")
