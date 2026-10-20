#include <gp4/gp4.h>

// Stores 1 into the byte at ebx+0x1c (`mov byte [ebx+0x1c], 1; ret`); flags untouched.
// FUNCTION: GP4 0x00531737
void __cdecl FUN_00531737(gp4::Regs* r) {
    uint8_t* rec = reinterpret_cast<uint8_t*>(static_cast<uintptr_t>(r->ebx));
    rec[0x1c] = 1u;
}
GP4_IMPL(0x00531737, FUN_00531737, "regs(ebx:ptr[0x40]:bytes) -> ()")
