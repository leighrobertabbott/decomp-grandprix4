#include <gp4/gp4.h>
#include <gp4/flags.h>

// Resets the 0x22 (34) entries of the 0x14-byte table starting at 0x838ec1:
// bit 0 of the flag byte +0x10 is set, words +4/+6 become 0 and words +8/+10
// become 1. ebx and ecx are saved/restored; only the flags of the final
// `dec ecx` (CF kept from the `add ebx, 0x14` before it) are left behind.

// FUNCTION: GP4 0x0054f753
void __cdecl FUN_0054f753(gp4::Regs* r) {
    uint32_t entry = 0x00838ec1u;
    uint32_t flags = 0;
    for (uint32_t left = 0x22; left != 0; --left) {
        GP4_FIELD(uint8_t, entry, 0x10) |= 1;
        GP4_FIELD(uint16_t, entry, 4) = 0;
        GP4_FIELD(uint16_t, entry, 6) = 0;
        GP4_FIELD(uint16_t, entry, 8) = 1;
        GP4_FIELD(uint16_t, entry, 10) = 1;
        flags = gp4::flags::add32(entry, 0x14u);
        entry += 0x14u;
        flags = gp4::flags::dec32(left, flags);
    }
    r->eflags = gp4::flags::merge(r->eflags, flags);
}
GP4_IMPL(0x0054f753, FUN_0054f753, "regs() -> (cf, pf, af, zf, sf, of); globals=fuzz:i32")
