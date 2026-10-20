#include <gp4/gp4.h>
#include <gp4/flags.h>

// Counts the 22 per-driver flag bytes at 0x007c2a48: eax = number with bit 7 set and bit 6 clear,
// edx = number with both bits set. ebx and ecx are saved and restored. The flags are those of the
// final `dec ecx` (1 -> 0) with CF clear (the loop's `test` cleared it and inc/dec keep it).
// FUNCTION: GP4 0x0052ceef
void __cdecl FUN_0052ceef(gp4::Regs* r) {
    const uint8_t* bytes = GP4_ARRAY(uint8_t, 0x007c2a48);
    uint32_t single = 0u;
    uint32_t both = 0u;
    for (uint32_t i = 0u; i != 0x16u; ++i) {
        const uint8_t b = bytes[i];
        if ((b & 0x80u) != 0u) {
            if ((b & 0x40u) == 0u) {
                single += 1u;
            } else {
                both += 1u;
            }
        }
    }
    r->eax = single;
    r->edx = both;
    r->eflags = gp4::flags::merge(r->eflags, gp4::flags::dec32(1u, 0u));
}
GP4_IMPL(0x0052ceef, FUN_0052ceef, "regs() -> (eax, edx, cf, pf, af, zf, sf, of); globals=layout=drv_005117d3")
