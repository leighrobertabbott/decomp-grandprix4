#include <gp4/gp4.h>

// Stores 0xff into the byte at ebx+0x26 (`mov byte [ebx+0x26], 0xff; ret`); flags untouched.
// FUNCTION: GP4 0x00531711
void __cdecl FUN_00531711(gp4::Regs* r) {
    uint8_t* rec = reinterpret_cast<uint8_t*>(static_cast<uintptr_t>(r->ebx));
    rec[0x26] = 0xffu;
}
GP4_IMPL(0x00531711, FUN_00531711, "regs(ebx:ptr[0x40]:bytes) -> ()")
