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

// (a * b) >> 14 with a 64-bit product, low 32 bits kept (imul / shrd eax, edx, 14)
static inline uint32_t mulShift14(uint32_t a, uint32_t b) {
    const int64_t product = static_cast<int64_t>(static_cast<int32_t>(a)) * static_cast<int64_t>(static_cast<int32_t>(b));
    return static_cast<uint32_t>(static_cast<uint64_t>(product) >> 14);
}

static inline uint32_t signExtend16(uint32_t v) {
    return static_cast<uint32_t>(static_cast<int32_t>(static_cast<int16_t>(v)));
}

// FUNCTION: GP4 0x0040e37e
// Three 16-bit angles (ax = a, bx = b, cx = c) through the table at 0x0040119c:
// products of the form S/C(a) * S/C(b) * S/C(c) >> 28 come back in ebx, ecx, edx,
// and the last table value (of c) in eax.
void __cdecl FUN_0040e37e(gp4::Regs* r) {
    const uint32_t a = r->eax & 0xffffu;
    const uint32_t b = r->ebx & 0xffffu;
    const uint32_t c = r->ecx & 0xffffu;
    uint32_t flags;

    const uint32_t cosC = signExtend16(lookup(r, 0x4000u - c, &flags));
    const uint32_t sinB = signExtend16(lookup(r, b, &flags));
    const uint32_t cosA = signExtend16(lookup(r, 0x4000u - a, &flags));
    const uint32_t sinA = signExtend16(lookup(r, a, &flags));

    const uint32_t p1 = mulShift14(mulShift14(cosA, sinB), cosC);
    const uint32_t cosB = signExtend16(lookup(r, 0x4000u - b, &flags));
    const uint32_t p2 = mulShift14(cosB, cosC);
    const uint32_t p3 = mulShift14(mulShift14(sinA, sinB), cosC);
    const uint32_t sinC = signExtend16(lookup(r, c, &flags));

    r->eax = sinC;
    r->ebx = p1;
    r->ecx = p2;
    r->edx = p3;
    r->eflags = (r->eflags & ~0x8d5u) | (flags & 0x8d5u);
}
GP4_IMPL(0x0040e37e, FUN_0040e37e, "regs(eax:u32, ebx:u32, ecx:u32) -> (eax, ebx, ecx, edx, cf, pf, af, zf, sf, of)")
