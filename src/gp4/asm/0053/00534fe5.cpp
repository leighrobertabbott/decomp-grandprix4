#include <gp4/gp4.h>

// Loads the dword at ebx + 4 into eax and stores it, unaligned, at DAT_00828293.
// FUNCTION: GP4 0x00534fe5
void __cdecl FUN_00534fe5(gp4::Regs* r) {
    const uint32_t value = GP4_FIELD(uint32_t, r->ebx, 4);
    GP4_GLOBAL(uint32_t, 0x00828293) = value;
    r->eax = value;
}
GP4_IMPL(0x00534fe5, FUN_00534fe5, "regs(ebx:ptr[0x40]:bytes) -> (eax); globals=fuzz:i32")
