#include <gp4/gp4.h>
#include <gp4/flags.h>

// Carry-flag test: CF = 1 only when the byte 0x007aafe0 differs from 0xff and the byte 0x007aafe1
// equals 0xff (`stc`); every other case ends with `clc`. The other flags are those of the last
// `cmp byte, 0xff` executed.
// FUNCTION: GP4 0x0054b094
void __cdecl FUN_0054b094(gp4::Regs* r) {
    const uint8_t first = GP4_GLOBAL(uint8_t, 0x007aafe0);
    uint32_t flags = gp4::flags::sub8(first, 0xffu);
    uint32_t carry = 0u;
    if (first != 0xffu) {
        const uint8_t second = GP4_GLOBAL(uint8_t, 0x007aafe1);
        flags = gp4::flags::sub8(second, 0xffu);
        if (second == 0xffu) {
            carry = 1u;
        }
    }
    r->eflags = gp4::flags::merge(r->eflags, (flags & ~gp4::flags::CF) | carry);
}
GP4_IMPL(0x0054b094, FUN_0054b094, "regs() -> (cf, pf, af, zf, sf, of); globals=fuzz:i32")
