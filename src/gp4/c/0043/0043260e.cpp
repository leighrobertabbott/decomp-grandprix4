#include <gp4/gp4.h>

// FUNCTION: GP4 0x0043260e
void __cdecl FUN_0043260e(gp4::Regs* r) {
    uint8_t* record = reinterpret_cast<uint8_t*>(static_cast<uintptr_t>(r->esi));
    const uint32_t self = r->esi;

    // esi is the car record; the two globals hold the player / focus car pointers
    if (GP4_GLOBAL(uint8_t, 0x007aafd4) == 0u || self != GP4_GLOBAL(uint32_t, 0x0066d828)) {
        if (self == GP4_GLOBAL(uint32_t, 0x0066d820)) {
            if (GP4_GLOBAL(int16_t, 0x007c2eb8) >= 2) {
                GP4_GLOBAL(uint32_t, 0x0066d83c) = GP4_GLOBAL(uint32_t, 0x0066d814);
            }
        } else {
            GP4_FIELD(uint8_t, record, 0x8d) |= 0x40u;
        }
    }

    // tail: call 0x418097 leaves the flags (and the other registers) as the caller sees them
    gp4::Regs callee = *r;
    gp4::call_regs(0x00418097, callee);
    *r = callee;
}
GP4_IMPL(0x0043260e, FUN_0043260e, "regs(esi:ptr[0x106]:bytes) -> (cf, pf, af, zf, sf, of); globals=layout=state_0043260e")
