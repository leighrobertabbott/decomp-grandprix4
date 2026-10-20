#include <gp4/gp4.h>

static inline uint32_t parityFlag(uint32_t value) {
    uint32_t parity = value & 0xffu;
    parity ^= parity >> 4;
    parity ^= parity >> 2;
    parity ^= parity >> 1;
    return (parity & 1u) == 0u ? 4u : 0u;
}

// Flags of `neg value` (= cmp 0, value).
static inline uint32_t negFlags(uint32_t value) {
    const uint32_t result = 0u - value;
    return (value != 0u ? 1u : 0u)
        | parityFlag(result)
        | (((value ^ result) & 0x10u) != 0u ? 0x10u : 0u)
        | (result == 0u ? 0x40u : 0u)
        | ((result & 0x80000000u) != 0u ? 0x80u : 0u)
        | (value == 0x80000000u ? 0x800u : 0u);
}

// Sine of a 16-bit angle through the table routine 0x0040119c, sign-extended (movsx eax, ax).
static inline int32_t sineOf(gp4::Regs* r, uint16_t angle) {
    gp4::Regs callee = *r;
    callee.eax = (callee.eax & 0xffff0000u) | angle;
    gp4::call_regs(0x0040119c, callee);
    return static_cast<int16_t>(callee.eax);
}

// Cosine: sine of (0x4000 - angle) (neg ax; add ax, 0x4000).
static inline int32_t cosineOf(gp4::Regs* r, uint16_t angle) {
    return sineOf(r, static_cast<uint16_t>(0x4000u - angle));
}

// FUNCTION: GP4 0x0041d8ab
// Stores sin/cos (Q14) of the three angles at 0x00638032 / 0x00638036 / 0x0063803a into
// 0x00623ee0/edc, 0x00623ee8/ee4 and 0x00623ef0/eec, builds the rotation matrix with 0x0040193c,
// then stores sin/cos of the word at esi+0x0e into 0x00623ef8 / 0x00623ef4 and the negated
// cosine into 0x00623f0c. Leaves eax = -cos, the flags of that neg, and edx/ebp as 0x0040193c
// left them.
void __cdecl FUN_0041d8ab(gp4::Regs* r) {
    uint8_t* body = reinterpret_cast<uint8_t*>(static_cast<uintptr_t>(r->esi));

    uint16_t angle = GP4_GLOBAL(uint16_t, 0x00638032);
    GP4_GLOBAL(int32_t, 0x00623ee0) = sineOf(r, angle);
    GP4_GLOBAL(int32_t, 0x00623edc) = cosineOf(r, angle);

    angle = GP4_GLOBAL(uint16_t, 0x00638036);
    GP4_GLOBAL(int32_t, 0x00623ee8) = sineOf(r, angle);
    GP4_GLOBAL(int32_t, 0x00623ee4) = cosineOf(r, angle);

    angle = GP4_GLOBAL(uint16_t, 0x0063803a);
    GP4_GLOBAL(int32_t, 0x00623ef0) = sineOf(r, angle);
    GP4_GLOBAL(int32_t, 0x00623eec) = cosineOf(r, angle);

    gp4::Regs matrix = *r;
    gp4::call_regs(0x0040193c, matrix);
    r->eax = matrix.eax;
    r->edx = matrix.edx;
    r->ebp = matrix.ebp;
    r->eflags = (r->eflags & ~0x8d5u) | (matrix.eflags & 0x8d5u);

    angle = GP4_FIELD(uint16_t, body, 0x0e);
    GP4_GLOBAL(int32_t, 0x00623ef8) = sineOf(r, angle);

    angle = GP4_FIELD(uint16_t, body, 0x0e);
    const int32_t cosine = cosineOf(r, angle);
    GP4_GLOBAL(int32_t, 0x00623ef4) = cosine;

    const uint32_t negated = 0u - static_cast<uint32_t>(GP4_GLOBAL(int32_t, 0x00623ef4));
    GP4_GLOBAL(uint32_t, 0x00623f0c) = negated;

    r->eax = negated;
    r->eflags = (r->eflags & ~0x8d5u) | negFlags(static_cast<uint32_t>(cosine));
}
GP4_IMPL(0x0041d8ab, FUN_0041d8ab, "regs(esi:ptr[0x18]:bytes) -> (eax, edx, ebp, cf, pf, af, zf, sf, of); globals=fuzz:i32")
