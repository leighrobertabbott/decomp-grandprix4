#include <gp4/gp4.h>

static inline uint32_t parityFlag(uint32_t value) {
    value &= 0xffu;
    value ^= value >> 4;
    value ^= value >> 2;
    value ^= value >> 1;
    return (value & 1u) == 0u ? 4u : 0u;
}

static inline uint32_t subtractionFlags32(uint32_t left, uint32_t right) {
    const uint32_t result = left - right;
    return (left < right ? 1u : 0u)
        | parityFlag(result)
        | (((left ^ right ^ result) & 0x10u) != 0u ? 0x10u : 0u)
        | (result == 0u ? 0x40u : 0u)
        | ((result & 0x80000000u) != 0u ? 0x80u : 0u)
        | (((left ^ right) & (left ^ result) & 0x80000000u) != 0u ? 0x800u : 0u);
}

// FUNCTION: GP4 0x00413139
// eax = the incoming value (kept in ebx), edi = the 0x160-byte ring record that 0x413090 inspects.
// 0x413090 returns CF clear when the record has no table entry: eax stays, the flags are those of 0x413090.
// With CF set it returns a magnitude m in eax: v = ((m * [0x601334]) >> 18) * [0x6224c9] >> 14 (two 32x32->64
// signed products, each shrd'ed down), limited to [0x6013ac] (signed), and eax = max(incoming, v) (signed);
// the flags are those of the final "cmp ebx, eax" (incoming against the limited v). ebx and edx are saved and
// restored; ecx comes back as 0x413090 left it.
void __cdecl FUN_00413139(gp4::Regs* r) {
    const int32_t incoming = static_cast<int32_t>(r->eax);

    gp4::Regs callee = *r;
    callee.st_in = 0;
    callee.st_out = 0;
    gp4::call_regs(0x00413090, callee);

    int32_t result = incoming;
    uint32_t flags = callee.eflags & 0x8d5u;

    if ((callee.eflags & 1u) != 0u) {
        const int64_t first = static_cast<int64_t>(static_cast<int32_t>(callee.eax))
            * static_cast<int64_t>(GP4_GLOBAL(int32_t, 0x00601334));
        const int32_t scaled = static_cast<int32_t>(static_cast<uint32_t>(static_cast<uint64_t>(first) >> 18));

        const int64_t second = static_cast<int64_t>(scaled)
            * static_cast<int64_t>(GP4_GLOBAL(int32_t, 0x006224c9));
        int32_t value = static_cast<int32_t>(static_cast<uint32_t>(static_cast<uint64_t>(second) >> 14));

        if (value > GP4_GLOBAL(int32_t, 0x006013ac)) {
            value = GP4_GLOBAL(int32_t, 0x006013ac);
        }

        flags = subtractionFlags32(static_cast<uint32_t>(incoming), static_cast<uint32_t>(value));
        if (incoming < value) {
            result = value;
        }
    }

    r->eax = static_cast<uint32_t>(result);
    r->ecx = callee.ecx;
    r->eflags = (r->eflags & ~0x8d5u) | flags;
}
GP4_IMPL(0x00413139, FUN_00413139, "regs(eax:u32, ecx:u32, edx:u32, edi:rec[0x8943a0,0x160,0..29]) -> (eax, ecx, cf, pf, af, zf, sf, of); globals=fuzz:i32+layout=ring_008943a0+words_00413090")
