#include <gp4/gp4.h>
#include <gp4/flags.h>

// Returns the (unaligned) dword at 0x00833cd7 in eax. When the byte flag at 0x00ae6740 is clear the
// value is repacked: bits 0..4 stay, bits 6..15 move down one place (bit 5 and the high half are
// dropped). Flags: those of `cmp byte [0xae6740], 0` when the flag is set, of the final `or` otherwise.
// FUNCTION: GP4 0x0054cd22
void __cdecl FUN_0054cd22(gp4::Regs* r) {
    uint32_t value = GP4_GLOBAL(uint32_t, 0x00833cd7);
    const uint8_t mode = GP4_GLOBAL(uint8_t, 0x00ae6740);
    uint32_t arith = gp4::flags::sub8(mode, 0u);
    if (mode == 0u) {
        const uint32_t high = (value & 0xffc0u) >> 1;
        value = (value & 0x1fu) | high;
        arith = gp4::flags::logic32(value);
    }
    r->eax = value;
    r->eflags = gp4::flags::merge(r->eflags, arith);
}
GP4_IMPL(0x0054cd22, FUN_0054cd22, "regs() -> (eax, cf, pf, af, zf, sf, of); globals=fuzz:i32")
