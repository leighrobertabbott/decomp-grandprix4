#include <gp4/gp4.h>

// FUNCTION: GP4 0x0040185a
// Fixed-point (Q14) dot product of two 3-vectors of 32-bit integers:
// DAT_00623e28 = (a0*b0 >> 14) + (a1*b1 >> 14) + (a2*b2 >> 14), each product taken
// as a signed 64-bit value and shifted with SHRD (low 32 bits only).
// The original leaves the last product's high half in edx, the partial sum of
// the first two terms in ebp and the flags of the final add.
void __cdecl FUN_0040185a(gp4::Regs* r) {
    const int64_t p0 = (int64_t)GP4_GLOBAL(int32_t, 0x00623e30) * (int64_t)GP4_GLOBAL(int32_t, 0x00623e48);
    const int64_t p1 = (int64_t)GP4_GLOBAL(int32_t, 0x00623e34) * (int64_t)GP4_GLOBAL(int32_t, 0x00623e4c);
    const int64_t p2 = (int64_t)GP4_GLOBAL(int32_t, 0x00623e38) * (int64_t)GP4_GLOBAL(int32_t, 0x00623e50);

    const uint32_t t0 = (uint32_t)((uint64_t)p0 >> 14);
    const uint32_t t1 = (uint32_t)((uint64_t)p1 >> 14);
    const uint32_t t2 = (uint32_t)((uint64_t)p2 >> 14);

    const uint32_t partial = t0 + t1;          // ebp
    const uint32_t sum = t2 + partial;         // eax

    GP4_GLOBAL(uint32_t, 0x00623e28) = sum;

    // flags of the final "add eax, ebp"
    uint32_t fl = r->eflags & ~0x8D5u;
    if (sum < partial) fl |= 0x001u;                                   // CF
    uint32_t b = sum & 0xFFu;
    b ^= b >> 4; b ^= b >> 2; b ^= b >> 1;
    if (!(b & 1)) fl |= 0x004u;                                        // PF
    if ((t2 ^ partial ^ sum) & 0x10u) fl |= 0x010u;                    // AF
    if (sum == 0) fl |= 0x040u;                                        // ZF
    if (sum & 0x80000000u) fl |= 0x080u;                               // SF
    if (((t2 ^ sum) & (partial ^ sum)) & 0x80000000u) fl |= 0x800u;    // OF

    r->eax = sum;
    r->edx = (uint32_t)((uint64_t)p2 >> 32);
    r->ebp = partial;
    r->eflags = fl;
}
GP4_IMPL(0x0040185a, FUN_0040185a, "regs() -> (eax, edx, ebp, cf, pf, af, zf, sf, of); globals=fuzz:i32")
