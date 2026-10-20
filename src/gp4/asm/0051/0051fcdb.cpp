#include <gp4/gp4.h>
#include <gp4/flags.h>

// FUNCTION: GP4 0x0051fcdb
// Binary to packed BCD of the low byte of eax: hundreds in bits 8..11, tens in 4..7, units in
// 0..3 (e.g. 0xfe -> 0x254). edx is saved and restored. Flags are those of the final `or dl, al`.
void __cdecl FUN_0051fcdb(gp4::Regs* r) {
    int32_t value = (int32_t)(r->eax & 0xffu);
    uint32_t packed = 0;

    while (value >= 100) {
        value -= 100;
        packed += 0x100u;
    }
    while (value >= 10) {
        value -= 10;
        packed += 0x10u;
    }
    const uint32_t low = (packed | (uint32_t)value) & 0xffu;
    packed = (packed & ~0xffu) | low;

    r->eax = packed;
    r->eflags = gp4::flags::merge(r->eflags, gp4::flags::logic8(low));
}
GP4_IMPL(0x0051fcdb, FUN_0051fcdb, "regs(eax:u32) -> (eax, cf, pf, af, zf, sf, of)")
