#include <gp4/gp4.h>

// Reads two bytes at esi (lodsb x2, esi += 2) as a big-endian word: ebx = (b0 << 8) | b1, ax
// = the same word (eax's upper half kept). Then calls the empty routine 0x0054d707. Flags
// untouched.
// FUNCTION: GP4 0x0054d334
void __cdecl FUN_0054d334(gp4::Regs* r) {
    const uint8_t* src = reinterpret_cast<const uint8_t*>(static_cast<uintptr_t>(r->esi));
    const uint32_t first = src[0];
    const uint32_t second = src[1];
    const uint32_t word = (first << 8) | second;
    r->esi = r->esi + 2u;
    r->eax = (r->eax & 0xffff0000u) | word;
    r->ebx = word;
    gp4::cdecl_<0x0054d707, void>();
}
GP4_IMPL(0x0054d334, FUN_0054d334, "regs(eax:u32, esi:ptr[0x40]:bytes) -> (eax, ebx, esi)")
