#include <gp4/gp4.h>

// Stores 2 into the byte at ebx+0x1c (`mov byte [ebx+0x1c], 2; ret`); flags untouched.
// FUNCTION: GP4 0x00531732
void __cdecl FUN_00531732(gp4::Regs* r) {
    uint8_t* rec = reinterpret_cast<uint8_t*>(static_cast<uintptr_t>(r->ebx));
    rec[0x1c] = 2u;
}
GP4_IMPL(0x00531732, FUN_00531732, "regs(ebx:ptr[0x40]:bytes) -> ()")
