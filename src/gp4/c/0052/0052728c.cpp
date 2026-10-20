#include <gp4/gp4.h>

// 0x0040119c: ax = table value of the 16-bit angle in ax; leaves the flags of its final add.
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

// FUNCTION: GP4 0x0052728c
// Three 16-bit angles at 0x00679b90 / 0x00679b5e / 0x00891510: table value of each into 0x008755dc /
// 0x008755e4 / 0x008755ec and the table value of (0x4000 - angle) into 0x008755d8 / 0x008755e0 /
// 0x008755e8. Returns the last first-value in eax and the last second-value in edx.
void __cdecl FUN_0052728c(gp4::Regs* r) {
    uint32_t flags;
    int32_t first = 0;
    int32_t second = 0;

    uint32_t angle = GP4_GLOBAL(uint16_t, 0x00679b90);
    first = signExtend16(lookup(r, angle, &flags));
    GP4_GLOBAL(int32_t, 0x008755dc) = first;
    second = signExtend16(lookup(r, 0x4000u - angle, &flags));
    GP4_GLOBAL(int32_t, 0x008755d8) = second;

    angle = GP4_GLOBAL(uint16_t, 0x00679b5e);
    first = signExtend16(lookup(r, angle, &flags));
    GP4_GLOBAL(int32_t, 0x008755e4) = first;
    second = signExtend16(lookup(r, 0x4000u - angle, &flags));
    GP4_GLOBAL(int32_t, 0x008755e0) = second;

    angle = GP4_GLOBAL(uint16_t, 0x00891510);
    first = signExtend16(lookup(r, angle, &flags));
    GP4_GLOBAL(int32_t, 0x008755ec) = first;
    second = signExtend16(lookup(r, 0x4000u - angle, &flags));
    GP4_GLOBAL(int32_t, 0x008755e8) = second;

    r->eax = static_cast<uint32_t>(first);
    r->edx = static_cast<uint32_t>(second);
    r->eflags = (r->eflags & ~0x8d5u) | (flags & 0x8d5u);
}
GP4_IMPL(0x0052728c, FUN_0052728c, "regs() -> (eax, edx, cf, pf, af, zf, sf, of); globals=fuzz:i32")
