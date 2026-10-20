#include <gp4/gp4.h>
#include <gp4/x87.h>

static inline uint32_t parityFlag(uint32_t value) {
    uint32_t parity = value & 0xffu;
    parity ^= parity >> 4;
    parity ^= parity >> 2;
    parity ^= parity >> 1;
    return (parity & 1u) == 0u ? 4u : 0u;
}

// flags of "add left, right" (dword)
static inline uint32_t addFlags(uint32_t left, uint32_t right) {
    const uint32_t result = left + right;
    return parityFlag(result)
        | (((left ^ right ^ result) & 0x10u) != 0u ? 0x10u : 0u)
        | (result == 0u ? 0x40u : 0u)
        | ((result & 0x80000000u) != 0u ? 0x80u : 0u)
        | ((~(left ^ right) & (left ^ result) & 0x80000000u) != 0u ? 0x800u : 0u)
        | (result < left ? 1u : 0u);
}

// (value << 3) sign-extended to 64 bits, shifted left by 8 (cdq; shld edx,eax,8; shl eax,8),
// divided by the divisor with a single idiv.
static inline int32_t scaledQuotient(int32_t value, int32_t divisor, int32_t* remainder) {
    const int32_t shifted = static_cast<int32_t>(static_cast<uint32_t>(value) << 3);
    return gp4::idiv64(static_cast<int64_t>(shifted) << 8, divisor, remainder);
}

// Same arithmetic as 0x00419b84 on the already summed values (after 0x00419eeb and 0x00419f9e):
// [0x00637fc8] and [0x00637fd0] are divided in 24.8 fixed point by max(word +0xb2 of the body, 0x200);
// the first quotient plus [0x00637f98] and [0x00607b00] goes to [0x00637f60], the second one (plus
// [0x00637fa0] while byte [0x00622485] is zero and the body is not in the 0x14280 / angle < 0x1d5
// state) plus [0x00607b08] to [0x00637f64]. Both are then handed to 0x00401d41 together with the
// two vectors -[0x00623edc] * k and [0x00623ee0] * k (k = [0x006f29f4]); the two x87 results are
// rounded into [0x00638ce4] and [0x00638cec]. ecx = first sum, ebp = eax = second sum, edx = the
// remainder of the second division, ebx as left by 0x00419f9e; the flags are those of the last
// `add eax, [0x00607b08]`.
// FUNCTION: GP4 0x00419c10
void __cdecl FUN_00419c10(gp4::Regs* r) {
    using namespace gp4::x87;

    gp4::Regs c = *r;
    c.st_in = 0;
    c.st_out = 0;
    c.st_xin_mask = 0;
    c.st_xout_mask = 0;
    gp4::call_regs(0x00419eeb, c);
    gp4::call_regs(0x00419f9e, c);

    uint8_t* const body = reinterpret_cast<uint8_t*>(static_cast<uintptr_t>(c.esi));
    int32_t divisor = GP4_FIELD(int16_t, body, 0xb2);
    if (divisor < 0x200) {
        divisor = 0x200;
    }

    int32_t remainder;
    const int32_t firstQuotient = scaledQuotient(GP4_GLOBAL(int32_t, 0x00637fc8), divisor, &remainder);
    const uint32_t first = static_cast<uint32_t>(firstQuotient) + GP4_GLOBAL(uint32_t, 0x00637f98)
        + GP4_GLOBAL(uint32_t, 0x00607b00);

    uint32_t second = static_cast<uint32_t>(scaledQuotient(GP4_GLOBAL(int32_t, 0x00637fd0), divisor, &remainder));
    if (GP4_GLOBAL(uint8_t, 0x00622485) == 0u
        && ((GP4_FIELD(uint32_t, body, 0x40) & 0x14280u) == 0u || GP4_FIELD(int16_t, body, 0x9a) >= 0x1d5)) {
        second += GP4_GLOBAL(uint32_t, 0x00637fa0);
    }
    const uint32_t offset = GP4_GLOBAL(uint32_t, 0x00607b08);
    const uint32_t flags = addFlags(second, offset);
    second += offset;

    GP4_GLOBAL(uint32_t, 0x00637f60) = first;
    GP4_GLOBAL(uint32_t, 0x00637f64) = second;

    const Ext k = ext(GP4_GLOBAL(double, 0x006f29f4));
    const Ext v1 = fchs(ext_i32(GP4_GLOBAL(int32_t, 0x00623edc))) * k;
    const Ext v2 = ext_i32(GP4_GLOBAL(int32_t, 0x00623ee0)) * k;
    const Ext s1 = ext_i32(GP4_GLOBAL(int32_t, 0x00637f64));
    const Ext s0 = ext_i32(GP4_GLOBAL(int32_t, 0x00637f60));

    c.eax = second;
    c.ecx = first;
    c.edx = static_cast<uint32_t>(remainder);
    c.ebp = second;
    c.st_in = 4;
    c.st_out = 2;
    set_in_st(c, 0, s0);
    set_in_st(c, 1, s1);
    set_in_st(c, 2, v2);
    set_in_st(c, 3, v1);
    gp4::call_regs(0x00401d41, c);

    GP4_GLOBAL(int32_t, 0x00638ce4) = fistp32(out_st(c, 0));
    GP4_GLOBAL(int32_t, 0x00638cec) = fistp32(out_st(c, 1));

    r->eax = c.eax;
    r->ebx = c.ebx;
    r->ecx = c.ecx;
    r->edx = c.edx;
    r->esi = c.esi;
    r->edi = c.edi;
    r->ebp = c.ebp;
    r->eflags = (r->eflags & ~0x8d5u) | flags;
}
GP4_IMPL(0x00419c10, FUN_00419c10, "regs(esi:ptr[0x267]:bytes) -> (eax, ebx, ecx, edx, ebp, cf, pf, af, zf, sf, of); globals=fuzz:i32+layout=sums_00419f9e")
