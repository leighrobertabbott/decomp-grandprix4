#include <gp4/gp4.h>

// Copies 0x160 bytes (one track-segment record) from the buffer pointed to by [0x00874b1c] to the
// buffer pointed to by [0x00874b20] with a forward `rep movsb` (direction flag clear, as the game
// runs). Leaves ecx = 0, esi = source end, edi = destination end; the flags are untouched.
// FUNCTION: GP4 0x00554817
void __cdecl FUN_00554817(gp4::Regs* r) {
    const uint32_t dstAddr = GP4_GLOBAL(uint32_t, 0x00874b20);
    const uint32_t srcAddr = GP4_GLOBAL(uint32_t, 0x00874b1c);
    uint8_t* dst = reinterpret_cast<uint8_t*>(static_cast<uintptr_t>(dstAddr));
    const uint8_t* src = reinterpret_cast<const uint8_t*>(static_cast<uintptr_t>(srcAddr));
    for (uint32_t i = 0; i != 0x160u; ++i) {
        dst[i] = src[i];
    }
    r->ecx = 0;
    r->esi = srcAddr + 0x160u;
    r->edi = dstAddr + 0x160u;
}
GP4_IMPL(0x00554817, FUN_00554817, "regs() -> (ecx, esi, edi); globals=layout=recs_00554817")
