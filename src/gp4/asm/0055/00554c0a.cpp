#include <gp4/gp4.h>

// pushal; three forward `rep movsd` of 14 dwords (0x38 bytes) each, popal; ret.
// Copies the block at 0x871ce8 to 0x871cb0, 0x872ae8 to 0x872ab0 and 0x8738e8 to 0x8738b0
// (each destination ends exactly where its source begins). Registers and flags are untouched.

static inline void copy14(uint32_t dstAddr, uint32_t srcAddr) {
    uint32_t* dst = reinterpret_cast<uint32_t*>(static_cast<uintptr_t>(dstAddr));
    const uint32_t* src = reinterpret_cast<const uint32_t*>(static_cast<uintptr_t>(srcAddr));
    for (int left = 14; left != 0; --left) {
        *dst++ = *src++;
    }
}

// FUNCTION: GP4 0x00554c0a
void __cdecl FUN_00554c0a(gp4::Regs* r) {
    (void)r;
    copy14(0x00871cb0u, 0x00871ce8u);
    copy14(0x00872ab0u, 0x00872ae8u);
    copy14(0x008738b0u, 0x008738e8u);
}
GP4_IMPL(0x00554c0a, FUN_00554c0a, "regs() -> (); globals=fuzz:i32")
