#include <gp4/gp4.h>

// FUNCTION: GP4 0x00401b85
// Scales three Q14 fixed-point values by DAT_00623e28:
// DAT_00623e18/1c/20 = (DAT_00623e28 * DAT_00623e64/70/7c) >> 14, each product a
// signed 64-bit value shifted with SHRD (low 32 bits only). The original leaves
// the last shrd result in eax, the last product's high half in edx and the
// flags of the last shrd.
void __cdecl FUN_00401b85(gp4::Regs* r) {
    const int64_t scale = (int64_t)GP4_GLOBAL(int32_t, 0x00623e28);
    const int64_t p0 = scale * (int64_t)GP4_GLOBAL(int32_t, 0x00623e64);
    const int64_t p1 = scale * (int64_t)GP4_GLOBAL(int32_t, 0x00623e70);
    const int64_t p2 = scale * (int64_t)GP4_GLOBAL(int32_t, 0x00623e7c);

    GP4_GLOBAL(uint32_t, 0x00623e18) = (uint32_t)((uint64_t)p0 >> 14);
    GP4_GLOBAL(uint32_t, 0x00623e1c) = (uint32_t)((uint64_t)p1 >> 14);
    const uint32_t res = (uint32_t)((uint64_t)p2 >> 14);
    GP4_GLOBAL(uint32_t, 0x00623e20) = res;

    // flags of the final "shrd eax, edx, 14": CF = last bit shifted out, AF = 0,
    // OF = sign change versus the 13-bit shift, SF/ZF/PF from the result.
    const uint32_t prev = (uint32_t)((uint64_t)p2 >> 13);
    uint32_t fl = r->eflags & ~0x8D5u;
    if (prev & 1u) fl |= 0x001u;
    uint32_t p = res & 0xffu;
    p ^= p >> 4; p ^= p >> 2; p ^= p >> 1;
    if (!(p & 1u)) fl |= 0x004u;
    if (res == 0) fl |= 0x040u;
    if (res & 0x80000000u) fl |= 0x080u;
    if ((prev ^ res) & 0x80000000u) fl |= 0x800u;

    r->eax = res;
    r->edx = (uint32_t)((uint64_t)p2 >> 32);
    r->eflags = fl;
}
GP4_IMPL(0x00401b85, FUN_00401b85, "regs() -> (eax, edx, cf, pf, af, zf, sf, of); globals=fuzz:i32")
