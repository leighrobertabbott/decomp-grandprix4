#include <gp4/gp4.h>

// 0x00401315: eax = 16-bit angle (sign-extended) of the vector (edx, eax); ebx/ecx/edx preserved.
static inline uint32_t angleOf(const gp4::Regs* r, uint32_t first, uint32_t second, uint32_t* flags) {
    gp4::Regs c = *r;
    c.st_in = 0;
    c.st_out = 0;
    c.eax = first;
    c.edx = second;
    gp4::call_regs(0x00401315, c);
    *flags = c.eflags;
    return c.eax;
}

// 0x0040119c: ax = table value of the angle in ax; the upper half of eax is kept.
static inline uint32_t lookup(const gp4::Regs* r, uint32_t eax) {
    gp4::Regs c = *r;
    c.st_in = 0;
    c.st_out = 0;
    c.eax = eax;
    gp4::call_regs(0x0040119c, c);
    return c.eax;
}

// 0x004010f3: eax = integer square root of eax.
static inline uint32_t squareRoot(const gp4::Regs* r, uint32_t value) {
    gp4::Regs c = *r;
    c.st_in = 0;
    c.st_out = 0;
    c.eax = value;
    gp4::call_regs(0x004010f3, c);
    return c.eax;
}

// FUNCTION: GP4 0x0040e2f9
// Three angles of the 3D vector (ebx, ecx, edx) with the extra component eax:
//   eax = angle of (edx, ebx)                      (first atan2)
//   ebx = angle of (|b-or-d|, table(angle1) * ecx >> 14)
//   ecx = angle of (eax, sqrt(ebx^2 + ecx^2 + edx^2))
// edx comes back as the input eax; esi/edi are restored; the flags are those of the last
// call to 0x00401315. The upper half of eax at the 0x0040119c call is the sign extension of
// the first angle and is multiplied in unchanged on the second branch (as the original does).
void __cdecl FUN_0040e2f9(gp4::Regs* r) {
    const uint32_t a = r->eax;
    const uint32_t b = r->ebx;
    const uint32_t c = r->ecx;
    const uint32_t d = r->edx;
    uint32_t flags;

    const uint32_t angle1 = angleOf(r, b, d, &flags);
    const uint32_t high = angle1 & 0xffff0000u;
    uint16_t ax = static_cast<uint16_t>(angle1);
    if (static_cast<int16_t>(ax) < 0) {
        ax = static_cast<uint16_t>(0u - ax);
    }
    if (ax >= 0x4000u) {
        ax = static_cast<uint16_t>((0u - ax) + 0x8000u);
    }

    uint32_t scaled;
    uint32_t second;
    if (ax < 0x2000u) {
        const uint32_t value = lookup(r, high | ax);
        const uint32_t product = static_cast<uint32_t>(static_cast<int32_t>(static_cast<int16_t>(value))) * c;
        scaled = static_cast<uint32_t>(static_cast<int32_t>(product) >> 14);
        second = d;
    } else {
        ax = static_cast<uint16_t>(0x4000u - ax);
        const uint32_t value = lookup(r, high | ax);
        const uint32_t product = value * c;
        scaled = static_cast<uint32_t>(static_cast<int32_t>(product) >> 14);
        second = b;
    }
    if (static_cast<int32_t>(second) < 0) {
        second = 0u - second;
    }

    const uint32_t angle2 = angleOf(r, scaled, second, &flags);

    uint32_t sumSquares = b * b;
    sumSquares += c * c;
    sumSquares += d * d;
    const uint32_t root = squareRoot(r, sumSquares);
    const uint32_t angle3 = angleOf(r, root, a, &flags);

    r->eax = angle1;
    r->ebx = angle2;
    r->ecx = angle3;
    r->edx = a;
    r->eflags = (r->eflags & ~0x8d5u) | (flags & 0x8d5u);
}
GP4_IMPL(0x0040e2f9, FUN_0040e2f9, "regs(eax:i32[-0x7fffffff..0x7fffffff], ebx:i32[-0x7fffffff..0x7fffffff], ecx:u32, edx:i32[-0x7fffffff..0x7fffffff]) -> (eax, ebx, ecx, edx, cf, pf, af, zf, sf, of)")
