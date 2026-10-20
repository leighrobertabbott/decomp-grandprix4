#include <gp4/gp4.h>

// Mirror of 0x00549d55: copies 0x3a980 dwords (960000 bytes) from the buffer whose address is the
// dword at 0x00822260 to the buffer whose address is the dword at 0x00822258 (`rep movsd`,
// forward). pushal/popal preserve every register and no flag is written.
// FUNCTION: GP4 0x00549d6b
void __cdecl FUN_00549d6b(gp4::Regs* r) {
    (void)r;
    const uint32_t* src = reinterpret_cast<const uint32_t*>(static_cast<uintptr_t>(GP4_GLOBAL(uint32_t, 0x00822260)));
    uint32_t* dst = reinterpret_cast<uint32_t*>(static_cast<uintptr_t>(GP4_GLOBAL(uint32_t, 0x00822258)));

    for (uint32_t i = 0; i != 0x3a980u; ++i) {
        dst[i] = src[i];
    }
}
GP4_IMPL(0x00549d6b, FUN_00549d6b, "regs() -> (); cases=40; globals=fuzz:i32+layout=surfcopy_00549d55; wall=3600")
