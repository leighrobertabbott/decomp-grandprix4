#include <gp4/gp4.h>

static inline uint32_t parityFlag(uint32_t value) {
    uint32_t parity = value & 0xffu;
    parity ^= parity >> 4;
    parity ^= parity >> 2;
    parity ^= parity >> 1;
    return (parity & 1u) == 0u ? 4u : 0u;
}

// flags of "add left, right"
static inline uint32_t add32Flags(uint32_t left, uint32_t right) {
    const uint32_t result = left + right;
    return (result < left ? 1u : 0u)
        | parityFlag(result)
        | (((left ^ right ^ result) & 0x10u) != 0u ? 0x10u : 0u)
        | (result == 0u ? 0x40u : 0u)
        | ((result & 0x80000000u) != 0u ? 0x80u : 0u)
        | ((~(left ^ right) & (left ^ result) & 0x80000000u) != 0u ? 0x800u : 0u);
}

// Turns the offsets stored in a car record into absolute pointers: +0x1c and +0x1b8 are offsets
// into the 0x160-byte segment table (base 0x89423c), and the seven optional links
// (+0x114, +0x118, +0x110, +0x128, +0x12c, +0x130, +0x124; zero stays zero) are offsets into the
// car array (base 0x66feec). The flags are those of the last link: its test (zero) or its add.
// FUNCTION: GP4 0x0040b186
void __cdecl FUN_0040b186(gp4::Regs* r) {
    uint8_t* car = reinterpret_cast<uint8_t*>(static_cast<uintptr_t>(r->esi));

    GP4_FIELD(uint32_t, car, 0x1c) += 0x0089423cu;
    GP4_FIELD(uint32_t, car, 0x1b8) += 0x0089423cu;

    if (GP4_FIELD(uint32_t, car, 0x114) != 0u) GP4_FIELD(uint32_t, car, 0x114) += 0x0066feecu;
    if (GP4_FIELD(uint32_t, car, 0x118) != 0u) GP4_FIELD(uint32_t, car, 0x118) += 0x0066feecu;
    if (GP4_FIELD(uint32_t, car, 0x110) != 0u) GP4_FIELD(uint32_t, car, 0x110) += 0x0066feecu;
    if (GP4_FIELD(uint32_t, car, 0x128) != 0u) GP4_FIELD(uint32_t, car, 0x128) += 0x0066feecu;
    if (GP4_FIELD(uint32_t, car, 0x12c) != 0u) GP4_FIELD(uint32_t, car, 0x12c) += 0x0066feecu;
    if (GP4_FIELD(uint32_t, car, 0x130) != 0u) GP4_FIELD(uint32_t, car, 0x130) += 0x0066feecu;

    const uint32_t last = GP4_FIELD(uint32_t, car, 0x124);
    uint32_t flags = 0x44u;   // test dword,-1 of zero: ZF and PF
    if (last != 0u) {
        flags = add32Flags(last, 0x0066feecu);
        GP4_FIELD(uint32_t, car, 0x124) = last + 0x0066feecu;
    }
    r->eflags = (r->eflags & ~0x8d5u) | flags;
}
GP4_IMPL(0x0040b186, FUN_0040b186, "regs(esi:ptr[0x1c4]:layout=links_0040b186) -> (cf, pf, af, zf, sf, of)")
