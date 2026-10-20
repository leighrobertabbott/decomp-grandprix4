#include <gp4/gp4.h>
#include <gp4/flags.h>

// Marks entry (clamp([0x00894238], 1, 0x16) - 1) of the byte table 0x007c2a48 with bit 7 and
// stores clamp([0x00894239], 1, 0x11) - 1 to [0x007ad894]. Leaves eax = that value and the flags
// of the final `dec eax` (CF kept from the preceding `cmp eax, 0x11`).
// FUNCTION: GP4 0x0051493c
void __cdecl FUN_0051493c(gp4::Regs* r) {
    uint32_t first = GP4_GLOBAL(uint8_t, 0x00894238);
    if (first < 1u) {
        first = 1u;
    }
    if (first > 0x16u) {
        first = 0x16u;
    }
    first -= 1u;
    GP4_ARRAY(uint8_t, 0x007c2a48)[first] |= 0x80u;

    uint32_t second = GP4_GLOBAL(uint8_t, 0x00894239);
    if (second < 1u) {
        second = 1u;
    }
    const uint32_t cmpFlags = gp4::flags::sub32(second, 0x11u);
    if (second > 0x11u) {
        second = 0x11u;
    }
    const uint32_t value = second - 1u;
    GP4_GLOBAL(uint32_t, 0x007ad894) = value;

    r->eax = value;
    r->eflags = gp4::flags::merge(r->eflags, gp4::flags::dec32(second, cmpFlags));
}
GP4_IMPL(0x0051493c, FUN_0051493c, "regs() -> (eax, cf, pf, af, zf, sf, of); globals=fuzz:i32")
