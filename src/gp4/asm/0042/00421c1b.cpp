#include <gp4/gp4.h>

struct Unknown_00421c1b {
    uint8_t Unknown00[0xb2];
    int16_t UnknownB2;
};

static inline uint32_t parityFlag(uint32_t value) {
    uint32_t parity = value & 0xffu;
    parity ^= parity >> 4;
    parity ^= parity >> 2;
    parity ^= parity >> 1;
    return (parity & 1u) == 0u ? 4u : 0u;
}

// flags of "sar reg32, 5" under the pinned oracle model:
// CF = bit 4 of the source, AF = 0, OF = 0.
static inline uint32_t sar5Flags(int32_t value) {
    const int32_t result = value >> 5;
    return (((uint32_t)value >> 4) & 1u)
        | parityFlag((uint32_t)result)
        | (result == 0 ? 0x40u : 0u)
        | (result < 0 ? 0x80u : 0u);
}

// (value << 3) sign-extended to 64 bits, shifted left by 8 (cdq; shld edx,eax,8; shl eax,8),
// divided by the divisor with a single idiv.
static inline int32_t scaledQuotient(int32_t value, int32_t divisor, int32_t* remainder) {
    const int32_t shifted = (int32_t)((uint32_t)value << 3);
    return gp4::idiv64((int64_t)shifted << 8, divisor, remainder);
}

// Converts the six fixed-point values at 0x00637f98..0x00637fac to 0x00607fc4..0x00607fe4:
// the first three are divided by 32; the last three are divided by max(word at +0xb2, 0x200)
// in 24.8 fixed point and then by 32.
// FUNCTION: GP4 0x00421c1b
void __cdecl FUN_00421c1b(gp4::Regs* r) {
    const Unknown_00421c1b* body = reinterpret_cast<const Unknown_00421c1b*>(r->esi);

    GP4_GLOBAL(int32_t, 0x00607fc4) = GP4_GLOBAL(int32_t, 0x00637f98) >> 5;
    GP4_GLOBAL(int32_t, 0x00607fc8) = GP4_GLOBAL(int32_t, 0x00637f9c) >> 5;
    GP4_GLOBAL(int32_t, 0x00607fcc) = GP4_GLOBAL(int32_t, 0x00637fa0) >> 5;

    int32_t divisor = body->UnknownB2;
    if (divisor < 0x200) {
        divisor = 0x200;
    }

    int32_t remainder;
    int32_t quotient = scaledQuotient(GP4_GLOBAL(int32_t, 0x00637fa4), divisor, &remainder);
    GP4_GLOBAL(int32_t, 0x00607fdc) = quotient >> 5;

    quotient = scaledQuotient(GP4_GLOBAL(int32_t, 0x00637fa8), divisor, &remainder);
    GP4_GLOBAL(int32_t, 0x00607fe0) = quotient >> 5;

    quotient = scaledQuotient(GP4_GLOBAL(int32_t, 0x00637fac), divisor, &remainder);
    GP4_GLOBAL(int32_t, 0x00607fe4) = quotient >> 5;

    r->eax = (uint32_t)(quotient >> 5);
    r->ecx = (uint32_t)divisor;
    r->edx = (uint32_t)remainder;
    r->eflags = (r->eflags & ~0x8d5u) | sar5Flags(quotient);
}
GP4_IMPL(0x00421c1b, FUN_00421c1b, "regs(esi:ptr[0xba]:bytes) -> (eax, ecx, edx, cf, pf, af, zf, sf, of); globals=fuzz:i32")
