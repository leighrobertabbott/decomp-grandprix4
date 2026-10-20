#include <gp4/gp4.h>

static inline uint32_t parityFlag(uint32_t value) {
    uint32_t parity = value & 0xffu;
    parity ^= parity >> 4;
    parity ^= parity >> 2;
    parity ^= parity >> 1;
    return (parity & 1u) == 0u ? 4u : 0u;
}

// Flags of a 32-bit logic result (`or`): CF = OF = AF = 0.
static inline uint32_t logicFlags(uint32_t result) {
    return parityFlag(result) | (result == 0u ? 0x40u : 0u) | ((result >> 24) & 0x80u);
}

// (value * rate) >> 16 as `imul dword [rate]; shrd eax, edx, 16` leaves it: the low 32 bits of the
// signed 64-bit product shifted right by 16.
static inline uint32_t scaleByRate(int32_t value, int32_t rate) {
    return static_cast<uint32_t>(static_cast<uint64_t>(static_cast<int64_t>(value) * static_cast<int64_t>(rate)) >> 16);
}

// FUNCTION: GP4 0x0040e593
// Sibling of 0x0040e4ec for the second attitude block: integrates the angular rates 0x006081ec / f0 / f4
// over the step 0x00607fa0 (Q16) into the vector 0x00623e30 / 34 / 38. A zero vector leaves the
// attitude alone (eax / ebx / ecx = the vector, edx = 0 and the flags of the `or`). Otherwise the vector
// becomes a quaternion (0x0040e46e, stored at 0x00625a8c..98), is composed with the rotation of the
// 16-bit angles 0x00607f52 / 4e / 56 (0x0040e0e5, 0x0040dff6) and converted back into three angles by
// 0x0040e230, whose low words replace 0x00607f52 / 4e / 56. The registers on exit are those the last
// callee leaves. 0x006097a8 is cleared on both paths.
void __cdecl FUN_0040e593(gp4::Regs* r) {
    GP4_GLOBAL(uint32_t, 0x00623e30) = scaleByRate(GP4_GLOBAL(int32_t, 0x006081ec), GP4_GLOBAL(int32_t, 0x00607fa0));
    GP4_GLOBAL(uint32_t, 0x00623e34) = scaleByRate(GP4_GLOBAL(int32_t, 0x006081f0), GP4_GLOBAL(int32_t, 0x00607fa0));
    GP4_GLOBAL(uint32_t, 0x00623e38) = scaleByRate(GP4_GLOBAL(int32_t, 0x006081f4), GP4_GLOBAL(int32_t, 0x00607fa0));

    const uint32_t x = GP4_GLOBAL(uint32_t, 0x00623e30);
    const uint32_t y = GP4_GLOBAL(uint32_t, 0x00623e34);
    const uint32_t z = GP4_GLOBAL(uint32_t, 0x00623e38);
    const uint32_t any = x | y | z;

    if (any == 0u) {
        GP4_GLOBAL(uint8_t, 0x006097a8) = 0u;
        r->eax = x;
        r->ebx = y;
        r->ecx = z;
        r->edx = any;
        r->eflags = (r->eflags & ~0x8d5u) | logicFlags(any);
        return;
    }

    gp4::Regs c = *r;
    c.st_in = 0;
    c.st_out = 0;

    c.eax = x;
    c.ebx = y;
    c.ecx = z;
    c.edx = any;
    gp4::call_regs(0x0040e46e, c);

    GP4_GLOBAL(uint32_t, 0x00625a8c) = c.eax;
    GP4_GLOBAL(uint32_t, 0x00625a90) = c.ebx;
    GP4_GLOBAL(uint32_t, 0x00625a94) = c.ecx;
    GP4_GLOBAL(uint32_t, 0x00625a98) = c.edx;

    c.eax = (c.eax & 0xffff0000u) | GP4_GLOBAL(uint16_t, 0x00607f52);
    c.ebx = (c.ebx & 0xffff0000u) | GP4_GLOBAL(uint16_t, 0x00607f4e);
    c.ecx = (c.ecx & 0xffff0000u) | GP4_GLOBAL(uint16_t, 0x00607f56);
    gp4::call_regs(0x0040e0e5, c);
    gp4::call_regs(0x0040dff6, c);
    gp4::call_regs(0x0040e230, c);

    GP4_GLOBAL(uint16_t, 0x00607f52) = static_cast<uint16_t>(c.eax);
    GP4_GLOBAL(uint16_t, 0x00607f4e) = static_cast<uint16_t>(c.ebx);
    GP4_GLOBAL(uint16_t, 0x00607f56) = static_cast<uint16_t>(c.ecx);
    GP4_GLOBAL(uint8_t, 0x006097a8) = 0u;

    r->eax = c.eax;
    r->ebx = c.ebx;
    r->ecx = c.ecx;
    r->edx = c.edx;
    r->ebp = c.ebp;
    r->eflags = (r->eflags & ~0x8d5u) | (c.eflags & 0x8d5u);
}
GP4_IMPL(0x0040e593, FUN_0040e593, "regs() -> (eax, ebx, ecx, edx, ebp, cf, pf, af, zf, sf, of); globals=fuzz:i32")
