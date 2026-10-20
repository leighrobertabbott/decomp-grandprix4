#include <gp4/gp4.h>

// Stores 0 into the byte at ebx+0x1d (`mov byte [ebx+0x1d], 0; ret`); flags untouched.
// FUNCTION: GP4 0x00531721
void __cdecl FUN_00531721(gp4::Regs* r) {
    uint8_t* rec = reinterpret_cast<uint8_t*>(static_cast<uintptr_t>(r->ebx));
    rec[0x1d] = 0u;
}
GP4_IMPL(0x00531721, FUN_00531721, "regs(ebx:ptr[0x40]:bytes) -> ()")
