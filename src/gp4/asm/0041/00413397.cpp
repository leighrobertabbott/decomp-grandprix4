#include <gp4/gp4.h>

// EFLAGS bits: CF 0x1, PF 0x4, AF 0x10, ZF 0x40, SF 0x80, OF 0x800.
static inline uint32_t parityFlag(uint32_t value) {
    uint32_t p = value & 0xffu;
    p ^= p >> 4;
    p ^= p >> 2;
    p ^= p >> 1;
    return (p & 1u) == 0u ? 0x4u : 0u;
}

// Flags of "shrd eax, edx, count" (count 1..31) on the 64-bit value edx:eax,
// in the oracle's pinned model: CF = last bit out, AF = 0,
// OF = bit31(value shifted by count-1 XOR result).
static inline uint32_t shrdFlags(uint64_t pair, unsigned count) {
    const uint32_t result = (uint32_t)(pair >> count);
    const uint32_t previous = (uint32_t)(pair >> (count - 1u));
    uint32_t f = parityFlag(result);
    if (((pair >> (count - 1u)) & 1u) != 0u) f |= 0x1u;
    if (result == 0u) f |= 0x40u;
    if ((result & 0x80000000u) != 0u) f |= 0x80u;
    if (((previous ^ result) & 0x80000000u) != 0u) f |= 0x800u;
    return f;
}

// FUNCTION: GP4 0x00413397
void __cdecl FUN_00413397(gp4::Regs* r) {
    const uint8_t* body = (const uint8_t*)r->esi;

    // eax = [0x6034b0] * [0x6f2bec] >> 22
    const int64_t p1 = (int64_t)GP4_GLOBAL(int32_t, 0x006034b0) * (int64_t)GP4_GLOBAL(int32_t, 0x006f2bec);
    const int32_t scaled = (int32_t)(uint32_t)((uint64_t)p1 >> 22);

    // ebp = low32(scaled * (int16)[esi+0xb2])
    const int32_t half = (int32_t)*(const int16_t*)(body + 0xb2);
    const int64_t p2 = (int64_t)scaled * (int64_t)half;
    const int32_t ebp = (int32_t)(uint32_t)(uint64_t)p2;

    // eax = ([0x638ce0]^2) / [0x601334]
    const int32_t ecx = GP4_GLOBAL(int32_t, 0x00638ce0);
    int32_t rem1;
    const int32_t q1 = gp4::idiv64((int64_t)ecx * (int64_t)ecx, GP4_GLOBAL(int32_t, 0x00601334), &rem1);

    // eax = (q1 * ebp) >> 25
    const int64_t p3 = (int64_t)q1 * (int64_t)ebp;
    const int32_t s3 = (int32_t)(uint32_t)((uint64_t)p3 >> 25);

    // eax = (s3 * [0x60134c]) / [0x601334]
    const int64_t p4 = (int64_t)s3 * (int64_t)GP4_GLOBAL(int32_t, 0x0060134c);
    int32_t rem2;
    const int32_t q2 = gp4::idiv64(p4, GP4_GLOBAL(int32_t, 0x00601334), &rem2);

    // edx:eax = q2 * [0x6038d4]; shrd eax, edx, 14
    const int64_t p5 = (int64_t)q2 * (int64_t)GP4_GLOBAL(int32_t, 0x006038d4);
    const uint32_t result = (uint32_t)((uint64_t)p5 >> 14);

    GP4_GLOBAL(uint32_t, 0x006036a4) = result;
    GP4_GLOBAL(uint32_t, 0x006036a8) = result;

    r->eax = result;
    r->ecx = (uint32_t)ecx;
    r->edx = (uint32_t)((uint64_t)p5 >> 32);
    r->ebp = (uint32_t)ebp;
    r->eflags = (r->eflags & ~0x8d5u) | shrdFlags((uint64_t)p5, 14u);
}
GP4_IMPL(0x00413397, FUN_00413397, "regs(esi:ptr[0xba]:bytes) -> (eax, ecx, edx, ebp, cf, pf, af, zf, sf, of); globals=layout=state_00413397")
