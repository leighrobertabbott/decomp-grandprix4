#include <gp4/gp4.h>

// Right-aligned variant of 0x0054654a: copies the bytes of a text string (everything >= 0x14, up
// to the first control byte or NUL, which is not copied) into the surface buffer ending at
// base pointer 0x831568 + x + y * row pitch 0xb1bf40, i.e. starting `length` bytes earlier. The
// length comes from the string scanner 0x546249 (esi = text, length returned in eax). eax, esi and
// edi are saved and restored; the flags are never consumed (as for 0x0054654a).
// FUNCTION: GP4 0x005465bb
void __stdcall FUN_005465bb(int x, int y, const uint8_t* text) {
    gp4::Regs scan = {};
    scan.esi = static_cast<uint32_t>(reinterpret_cast<uintptr_t>(text));
    gp4::call_regs(0x00546249, scan);
    uint8_t* dst = reinterpret_cast<uint8_t*>(static_cast<uintptr_t>(
        static_cast<uint32_t>(y * GP4_GLOBAL(int, 0x00b1bf40) + x + GP4_GLOBAL(int, 0x00831568)) - scan.eax));
    while (*text >= 0x14u) {
        *dst++ = *text++;
    }
}
GP4_IMPL(0x005465bb, FUN_005465bb, "stdcall(i32[0..64], i32[0..8], ptr[0x200]:bytes) -> void; globals=fuzz:i32+layout=surf_0054654a")
