#include <gp4/gp4.h>

// Latches four register values into the block at 0x00833cf8..0x00833d04 and raises the byte flag
// at 0x00833cf4 (cleared again by 0x0054cd61): [0x833cf8] = ebx, [0x833cfc] = ebp,
// [0x833d00] = eax, [0x833d04] = ecx. No register or flag is changed.
// FUNCTION: GP4 0x0054cd42
void __cdecl FUN_0054cd42(gp4::Regs* r) {
    const uint32_t eax = r->eax;
    const uint32_t ebx = r->ebx;
    const uint32_t ecx = r->ecx;
    const uint32_t ebp = r->ebp;
    GP4_GLOBAL(uint8_t, 0x00833cf4) = 0xffu;
    GP4_GLOBAL(uint32_t, 0x00833cf8) = ebx;
    GP4_GLOBAL(uint32_t, 0x00833d00) = eax;
    GP4_GLOBAL(uint32_t, 0x00833cfc) = ebp;
    GP4_GLOBAL(uint32_t, 0x00833d04) = ecx;
}
GP4_IMPL(0x0054cd42, FUN_0054cd42, "regs(eax:u32, ebp:u32, ebx:u32, ecx:u32) -> (); globals=fuzz:i32")
