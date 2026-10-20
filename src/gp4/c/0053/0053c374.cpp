#include <gp4/gp4.h>

// Clears the byte 0x0082fd6a and the word 0x007ab00e, empties the key queue (0x0052d813 copies the write cursor
// over the read cursor and leaves it in edi) and clears the dword 0x0082fd9c. No instruction writes the flags.
// FUNCTION: GP4 0x0053c374
void __cdecl FUN_0053c374(gp4::Regs* r) {
    GP4_GLOBAL(uint8_t, 0x0082fd6a) = 0u;
    GP4_GLOBAL(uint16_t, 0x007ab00e) = 0u;
    gp4::call_regs(0x0052d813, *r);
    GP4_GLOBAL(uint32_t, 0x0082fd9c) = 0u;
}
GP4_IMPL(0x0053c374, FUN_0053c374, "regs() -> (edi); globals=fuzz:i32+layout=keyq_008245c0")
