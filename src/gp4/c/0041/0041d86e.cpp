#include <gp4/gp4.h>

// FUNCTION: GP4 0x0041d86e
// Scales three Q14 fixed-point values by DAT_00602fd8:
// DAT_00637f98/9c/a0 = (DAT_00602fd8 * DAT_00623e40/4c/58) >> 14, each product a
// signed 64-bit value shifted with SHRD (low 32 bits only). The original leaves
// the last shrd result in eax, the last product's high half in edx and the
// flags of the last shrd.
void __cdecl FUN_0041d86e(gp4::Regs* r) {
    const int64_t scale = (int64_t)GP4_GLOBAL(int32_t, 0x00602fd8);
    const int64_t p0 = scale * (int64_t)GP4_GLOBAL(int32_t, 0x00623e40);
    const int64_t p1 = scale * (int64_t)GP4_GLOBAL(int32_t, 0x00623e4c);
    const int64_t p2 = scale * (int64_t)GP4_GLOBAL(int32_t, 0x00623e58);

    GP4_GLOBAL(uint32_t, 0x00637f98) = (uint32_t)((uint64_t)p0 >> 14);
    GP4_GLOBAL(uint32_t, 0x00637f9c) = (uint32_t)((uint64_t)p1 >> 14);
    const uint32_t res = (uint32_t)((uint64_t)p2 >> 14);
    GP4_GLOBAL(uint32_t, 0x00637fa0) = res;

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
GP4_IMPL(0x0041d86e, FUN_0041d86e, "regs() -> (eax, edx, cf, pf, af, zf, sf, of); globals=fuzz:i32")
