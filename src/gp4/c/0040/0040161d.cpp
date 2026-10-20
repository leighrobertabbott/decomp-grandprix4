#include <gp4/gp4.h>

// 0x0040119c: ax = table lookup of the 16-bit angle in ax; leaves the flags of its final add.
static inline uint32_t lookup(const gp4::Regs* r, uint32_t angle, uint32_t* flags) {
    gp4::Regs c = *r;
    c.st_in = 0;
    c.st_out = 0;
    c.eax = angle & 0xffffu;
    gp4::call_regs(0x0040119c, c);
    *flags = c.eflags;
    return c.eax & 0xffffu;
}

static inline int32_t signExtend16(uint32_t v) {
    return static_cast<int32_t>(static_cast<int16_t>(v));
}

// FUNCTION: GP4 0x0040161d
// Three 16-bit angles at 0x00638dd2 / dd6 / dda: table value of each into 0x00623ec8 / ed0 / ed8 and
// the table value of (0x4000 - angle) into 0x00623ec4 / ecc / ed4. Returns the last first-value in eax.
void __cdecl FUN_0040161d(gp4::Regs* r) {
    uint32_t flags;
    int32_t first = 0;
    int32_t second = 0;

    uint32_t angle = GP4_GLOBAL(uint16_t, 0x00638dd2);
    first = signExtend16(lookup(r, angle, &flags));
    GP4_GLOBAL(int32_t, 0x00623ec8) = first;
    second = signExtend16(lookup(r, 0x4000u - angle, &flags));
    GP4_GLOBAL(int32_t, 0x00623ec4) = second;

    angle = GP4_GLOBAL(uint16_t, 0x00638dd6);
    first = signExtend16(lookup(r, angle, &flags));
    GP4_GLOBAL(int32_t, 0x00623ed0) = first;
    second = signExtend16(lookup(r, 0x4000u - angle, &flags));
    GP4_GLOBAL(int32_t, 0x00623ecc) = second;

    angle = GP4_GLOBAL(uint16_t, 0x00638dda);
    first = signExtend16(lookup(r, angle, &flags));
    GP4_GLOBAL(int32_t, 0x00623ed8) = first;
    second = signExtend16(lookup(r, 0x4000u - angle, &flags));
    GP4_GLOBAL(int32_t, 0x00623ed4) = second;

    r->eax = static_cast<uint32_t>(first);
    r->edx = static_cast<uint32_t>(second);
    r->eflags = (r->eflags & ~0x8d5u) | (flags & 0x8d5u);
}
GP4_IMPL(0x0040161d, FUN_0040161d, "regs() -> (eax, edx, cf, pf, af, zf, sf, of); globals=fuzz:i32")
