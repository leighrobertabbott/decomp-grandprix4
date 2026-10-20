#include <gp4/gp4.h>

static inline uint32_t parityFlag(uint32_t value) {
    uint32_t parity = value & 0xffu;
    parity ^= parity >> 4;
    parity ^= parity >> 2;
    parity ^= parity >> 1;
    return (parity & 1u) == 0u ? 4u : 0u;
}

// Flags of "shl reg32, 16" under the pinned oracle model: CF = last bit shifted out, AF = 0,
// OF = bit31((value << 15) ^ result).
static inline uint32_t shl16Flags(uint32_t value) {
    const uint32_t before = value << 15;
    const uint32_t result = value << 16;
    return ((before >> 31) & 1u)
        | parityFlag(result)
        | (result == 0u ? 0x40u : 0u)
        | ((result >> 31) << 7)
        | (((before ^ result) >> 31) << 11);
}

// (value * rate) >> 16 as `imul dword [rate]; shrd eax, edx, 16` leaves it: the low 32 bits of the
// signed 64-bit product shifted right by 16.
static inline uint32_t scaleByRate(int32_t value, int32_t rate) {
    return static_cast<uint32_t>(static_cast<uint64_t>(static_cast<int64_t>(value) * static_cast<int64_t>(rate)) >> 16);
}

// FUNCTION: GP4 0x0040e4ec
// Integrates the angular rates 0x00623eb8 / ec / c0 over the step 0x0060384c (Q16) into the
// vector 0x00623e30 / 34 / 38, turns it into a quaternion with 0x0040e46e (stored at 0x00625a8c..98),
// composes it with the rotation of the 16-bit angles 0x00638032 / 36 / 3a (0x0040e0e5, 0x0040dff6)
// and converts the product back into three angles with 0x0040e230, which are stored as the
// high words of 0x00638030 / 34 / 38 (shifted left by 16). The registers on exit are the shifted
// angles in eax / ebx / ecx, the root in edx, ebp from 0x0040dff6 and the flags of the last shl.
void __cdecl FUN_0040e4ec(gp4::Regs* r) {
    const int32_t rate = GP4_GLOBAL(int32_t, 0x0060384c);
    GP4_GLOBAL(uint32_t, 0x00623e30) = scaleByRate(GP4_GLOBAL(int32_t, 0x00623eb8), rate);
    GP4_GLOBAL(uint32_t, 0x00623e34) = scaleByRate(GP4_GLOBAL(int32_t, 0x00623ebc), GP4_GLOBAL(int32_t, 0x0060384c));
    GP4_GLOBAL(uint32_t, 0x00623e38) = scaleByRate(GP4_GLOBAL(int32_t, 0x00623ec0), GP4_GLOBAL(int32_t, 0x0060384c));

    gp4::Regs c = *r;
    c.st_in = 0;
    c.st_out = 0;

    c.eax = GP4_GLOBAL(uint32_t, 0x00623e30);
    c.ebx = GP4_GLOBAL(uint32_t, 0x00623e34);
    c.ecx = GP4_GLOBAL(uint32_t, 0x00623e38);
    gp4::call_regs(0x0040e46e, c);

    GP4_GLOBAL(uint32_t, 0x00625a8c) = c.eax;
    GP4_GLOBAL(uint32_t, 0x00625a90) = c.ebx;
    GP4_GLOBAL(uint32_t, 0x00625a94) = c.ecx;
    GP4_GLOBAL(uint32_t, 0x00625a98) = c.edx;

    c.eax = (c.eax & 0xffff0000u) | GP4_GLOBAL(uint16_t, 0x00638032);
    c.ebx = (c.ebx & 0xffff0000u) | GP4_GLOBAL(uint16_t, 0x00638036);
    c.ecx = (c.ecx & 0xffff0000u) | GP4_GLOBAL(uint16_t, 0x0063803a);
    gp4::call_regs(0x0040e0e5, c);
    gp4::call_regs(0x0040dff6, c);
    gp4::call_regs(0x0040e230, c);

    const uint32_t first = c.eax << 16;
    const uint32_t second = c.ebx << 16;
    const uint32_t third = c.ecx << 16;
    GP4_GLOBAL(uint32_t, 0x00638030) = first;
    GP4_GLOBAL(uint32_t, 0x00638034) = second;
    GP4_GLOBAL(uint32_t, 0x00638038) = third;

    r->eax = first;
    r->ebx = second;
    r->ecx = third;
    r->edx = c.edx;
    r->ebp = c.ebp;
    r->eflags = (r->eflags & ~0x8d5u) | shl16Flags(c.ecx);
}
GP4_IMPL(0x0040e4ec, FUN_0040e4ec, "regs() -> (eax, ebx, ecx, edx, ebp, cf, pf, af, zf, sf, of); globals=fuzz:i32")
