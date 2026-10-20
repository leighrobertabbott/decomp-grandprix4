#include <gp4/gp4.h>

// Clears the 0x1000 bytes at 0x007a97b0 (`rep stosb` of al = 0, direction flag clear) and sets
// dword 0x007a97ac to 1. Leaves al = 0, ecx = 0 and edi = 0x007aa7b0; the flags are untouched.
// FUNCTION: GP4 0x0054b8ac
void __cdecl FUN_0054b8ac(gp4::Regs* r) {
    uint32_t* block = reinterpret_cast<uint32_t*>(&GP4_GLOBAL(uint8_t, 0x007a97b0));
    for (uint32_t i = 0; i < 0x400u; ++i) {
        block[i] = 0u;
    }
    GP4_GLOBAL(uint32_t, 0x007a97ac) = 1u;
    r->eax &= 0xffffff00u;
    r->ecx = 0u;
    r->edi = 0x007a97b0u + 0x1000u;
}
GP4_IMPL(0x0054b8ac, FUN_0054b8ac, "regs(eax:u32) -> (eax, ecx, edi); globals=fuzz:i32")
