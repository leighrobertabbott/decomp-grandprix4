#include <gp4/gp4.h>

// Vertical text writer: copies the bytes >= 0x14 of a text string (up to the first control byte or
// NUL, which is not copied) down a column of the surface buffer, one byte per row. Start address =
// base pointer 0x831568 + x + (y - length) * row pitch 0xb1bf40, the length coming from the string
// scanner 0x546249 (esi = text, length returned in eax); each byte advances by one pitch. eax, esi
// and edi are saved and restored; the flags are never consumed.
// FUNCTION: GP4 0x005465f7
void __stdcall FUN_005465f7(int x, int y, const uint8_t* text) {
    gp4::Regs scan = {};
    scan.esi = static_cast<uint32_t>(reinterpret_cast<uintptr_t>(text));
    gp4::call_regs(0x00546249, scan);

    const uint32_t pitch = GP4_GLOBAL(uint32_t, 0x00b1bf40);
    uint32_t address = (uint32_t)y - scan.eax;
    address *= pitch;
    address += (uint32_t)x;
    address += GP4_GLOBAL(uint32_t, 0x00831568);

    uint8_t* dst = reinterpret_cast<uint8_t*>(static_cast<uintptr_t>(address));
    while (*text >= 0x14u) {
        *dst = *text++;
        dst += GP4_GLOBAL(uint32_t, 0x00b1bf40);
    }
}
GP4_IMPL(0x005465f7, FUN_005465f7, "stdcall(i32[0..64], i32[0..8], ptr[0x200]:bytes) -> void; globals=fuzz:i32+layout=surf_0054654a")
