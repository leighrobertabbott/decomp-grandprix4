#include <gp4/gp4.h>

// Clears the byte at ebx+0x1c (`mov byte ptr [ebx+0x1c], 0; ret`).
// FUNCTION: GP4 0x0053173c
void __cdecl FUN_0053173c(gp4::Regs* r) {
    uint8_t* p = reinterpret_cast<uint8_t*>(static_cast<uintptr_t>(r->ebx));
    p[0x1c] = 0;
}
GP4_IMPL(0x0053173c, FUN_0053173c, "regs(ebx:ptr[0x40]:bytes) -> ()")
