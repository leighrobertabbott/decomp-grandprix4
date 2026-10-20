#include <gp4/gp4.h>
#include <gp4/flags.h>

// FUNCTION: GP4 0x0051fd0c
// Binary to packed BCD of the low word of eax: ten-thousands in bits 16..19, thousands in 12..15,
// hundreds in 8..11, tens in 4..7, units in 0..3 (e.g. 0xffff -> 0x65535). edx is saved and
// restored. Flags are those of the final `or dl, al`.
void __cdecl FUN_0051fd0c(gp4::Regs* r) {
    int32_t value = static_cast<int32_t>(r->eax & 0xffffu);
    uint32_t packed = 0;

    while (value >= 10000) {
        value -= 10000;
        packed += 0x10000u;
    }
    while (value >= 1000) {
        value -= 1000;
        packed += 0x1000u;
    }
    while (value >= 100) {
        value -= 100;
        packed += 0x100u;
    }
    while (value >= 10) {
        value -= 10;
        packed += 0x10u;
    }
    const uint32_t low = (packed | static_cast<uint32_t>(value)) & 0xffu;
    packed = (packed & ~0xffu) | low;

    r->eax = packed;
    r->eflags = gp4::flags::merge(r->eflags, gp4::flags::logic8(low));
}
GP4_IMPL(0x0051fd0c, FUN_0051fd0c, "regs(eax:u32) -> (eax, cf, pf, af, zf, sf, of)")
