#include <gp4/gp4.h>

// Reads two bytes at esi (lodsb x2, esi += 2) as a big-endian word, stores that word (zero-extended
// to 32 bits) at the unaligned dword 0x00833cc5 and sets the byte 0x00833f74 to 0xff. eax = the
// word (movzx), the flags are untouched.
// FUNCTION: GP4 0x0054d2f2
void __cdecl FUN_0054d2f2(gp4::Regs* r) {
    const uint8_t* src = reinterpret_cast<const uint8_t*>(static_cast<uintptr_t>(r->esi));
    const uint32_t first = src[0];
    const uint32_t second = src[1];
    const uint32_t word = (first << 8) | second;
    r->esi = r->esi + 2u;
    r->eax = word;
    GP4_GLOBAL(uint32_t, 0x00833cc5) = word;
    GP4_GLOBAL(uint8_t, 0x00833f74) = 0xffu;
}
GP4_IMPL(0x0054d2f2, FUN_0054d2f2, "regs(esi:ptr[0x40]:bytes) -> (eax, esi); globals=fuzz:i32")
