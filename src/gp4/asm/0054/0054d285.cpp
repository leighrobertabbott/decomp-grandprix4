#include <gp4/gp4.h>

// `lodsb` then store: al = *esi++ (upper eax bytes kept) and the byte goes to 0x00833cf3.
// Flags are untouched.
// FUNCTION: GP4 0x0054d285
void __cdecl FUN_0054d285(gp4::Regs* r) {
    const uint8_t* p = reinterpret_cast<const uint8_t*>(static_cast<uintptr_t>(r->esi));
    const uint8_t al = *p;
    r->esi = r->esi + 1u;
    r->eax = (r->eax & 0xffffff00u) | al;
    GP4_GLOBAL(uint8_t, 0x00833cf3) = al;
}
GP4_IMPL(0x0054d285, FUN_0054d285, "regs(eax:u32, esi:ptr[0x40]:bytes) -> (eax, esi)")
