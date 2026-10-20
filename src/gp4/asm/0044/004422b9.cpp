#include <gp4/gp4.h>

// Clears the 0x7d00-byte buffer at 0x00630238 (`rep stosb` with the direction flag clear, as at every call in
// the game) and the dwords 0x00630230 and 0x0063022c. edi ends one past the buffer, ecx is 0, al is 0; flags are
// untouched.
// FUNCTION: GP4 0x004422b9
void __cdecl FUN_004422b9(gp4::Regs* r) {
    uint8_t* buffer = GP4_ARRAY(uint8_t, 0x00630238);
    for (uint32_t i = 0; i != 0x7d00u; ++i) {
        buffer[i] = 0;
    }
    GP4_GLOBAL(uint32_t, 0x00630230) = 0;
    GP4_GLOBAL(uint32_t, 0x0063022c) = 0;

    r->edi = 0x00630238u + 0x7d00u;
    r->ecx = 0;
    r->eax = r->eax & 0xffffff00u;
}
GP4_IMPL(0x004422b9, FUN_004422b9, "regs(eax:u32) -> (eax, ecx, edi); globals=fuzz:i32")
