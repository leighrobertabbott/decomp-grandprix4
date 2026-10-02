#include <gp4/gp4.h>

static inline uint32_t parityFlag(uint32_t value) {
    uint32_t parity = value & 0xffu;
    parity ^= parity >> 4;
    parity ^= parity >> 2;
    parity ^= parity >> 1;
    return (parity & 1u) == 0u ? 4u : 0u;
}

// FUNCTION: GP4 0x0040cfba
void __cdecl FUN_0040cfba(gp4::Regs* r) {
    // eax = (edx - eax) / 0x160, edx = remainder; the flags are those of the
    // add (idiv leaves them alone)
    const uint32_t left = 0u - r->eax;          // neg eax
    const uint32_t right = r->edx;
    const uint32_t sum = left + right;          // add eax, edx
    uint32_t flags = parityFlag(sum);
    if (sum < left) flags |= 0x001u;
    if (((left ^ right ^ sum) & 0x10u) != 0u) flags |= 0x010u;
    if (sum == 0u) flags |= 0x040u;
    if ((sum & 0x80000000u) != 0u) flags |= 0x080u;
    if ((((left ^ sum) & (right ^ sum)) & 0x80000000u) != 0u) flags |= 0x800u;

    const int32_t dividend = static_cast<int32_t>(sum);
    r->eax = static_cast<uint32_t>(dividend / 0x160);
    r->edx = static_cast<uint32_t>(dividend % 0x160);
    r->eflags = (r->eflags & ~0x8d5u) | flags;
}
GP4_IMPL(0x0040cfba, FUN_0040cfba, "regs(eax:i32, edx:i32) -> (eax, edx, cf, pf, af, zf, sf, of)")
