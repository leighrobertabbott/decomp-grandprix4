#include <gp4/gp4.h>

// FUNCTION: GP4 0x0041904e
// Clears every one of the three bytes at [esi+0x2b8..0x2ba] that equals (cl + 1).
// ebx/ecx are preserved; the flags are those of the final `cmp ebx, 3` (3 - 3 = 0): PF|ZF.
void __cdecl FUN_0041904e(gp4::Regs* r) {
    uint8_t* record = reinterpret_cast<uint8_t*>(static_cast<uintptr_t>(r->esi));
    const uint8_t value = static_cast<uint8_t>(r->ecx + 1u);
    for (uint32_t i = 0u; i < 3u; ++i) {
        if (GP4_FIELD(uint8_t, record, 0x2b8 + i) == value) {
            GP4_FIELD(uint8_t, record, 0x2b8 + i) = 0u;
        }
    }
    r->eflags = (r->eflags & ~0x8d5u) | 0x44u;
}
GP4_IMPL(0x0041904e, FUN_0041904e, "regs(ecx:u32, esi:ptr[0x2c2]:bytes) -> (cf, pf, af, zf, sf, of)")
