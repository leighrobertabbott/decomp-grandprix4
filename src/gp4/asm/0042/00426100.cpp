#include <gp4/gp4.h>

static inline uint32_t parityFlag(uint32_t value) {
    value &= 0xffu;
    value ^= value >> 4;
    value ^= value >> 2;
    value ^= value >> 1;
    return (value & 1u) == 0u ? 4u : 0u;
}

// Flags of the 32-bit `cmp value, 0` for a non-negative value: only PF and ZF can be set.
static inline uint32_t cmpZeroFlags(uint32_t value) {
    return parityFlag(value) | (value == 0u ? 0x40u : 0u);
}

// A negative value (eax) is scaled by 256, negated and multiplied by the cached reciprocal
// DAT_00603e8c (0x00426091 refreshes it for the body pair esi / ebx), then by DAT_00603e90, and
// shifted back by 8: eax = that result, edx = the first product (also stored to DAT_00603e94).
// A non-negative value leaves eax alone, zeroes edx and DAT_00603e94 and keeps the flags of
// `cmp ecx, 0`; the negative path ends with the flags of `sar eax, 8`. Everything else is saved
// and restored.
// FUNCTION: GP4 0x00426100
void __cdecl FUN_00426100(gp4::Regs* r) {
    const int32_t input = static_cast<int32_t>(r->eax);

    if (input >= 0) {
        GP4_GLOBAL(uint32_t, 0x00603e94) = 0u;
        r->edx = 0u;
        r->eflags = (r->eflags & ~0x8d5u) | cmpZeroFlags(static_cast<uint32_t>(input));
        return;
    }

    const uint32_t scaled = static_cast<uint32_t>(input) << 8;
    gp4::Regs callee = *r;
    callee.st_in = 0;
    callee.st_out = 0;
    callee.ecx = scaled;
    gp4::call_regs(0x00426091, callee);

    const int64_t first = static_cast<int64_t>(static_cast<int32_t>(0u - scaled)) * GP4_GLOBAL(int32_t, 0x00603e8c);
    const uint32_t ratio = static_cast<uint32_t>(static_cast<uint64_t>(first) >> 14);
    const int64_t second = static_cast<int64_t>(GP4_GLOBAL(int32_t, 0x00603e90)) * static_cast<int32_t>(ratio);
    const uint32_t blended = static_cast<uint32_t>(static_cast<uint64_t>(second) >> 14);
    const uint32_t result = static_cast<uint32_t>(static_cast<int32_t>(blended) >> 8);
    GP4_GLOBAL(uint32_t, 0x00603e94) = ratio;

    // sar eax, 8: CF = last bit shifted out, OF = AF = 0
    const uint32_t flags = ((blended >> 7) & 1u)
        | parityFlag(result)
        | (result == 0u ? 0x40u : 0u)
        | ((result >> 24) & 0x80u);

    r->eax = result;
    r->edx = ratio;
    r->eflags = (r->eflags & ~0x8d5u) | flags;
}
GP4_IMPL(0x00426100, FUN_00426100, "regs(eax:i32, esi:ptr[0xe8]:layout=rec_00426091, ebx:ptr[0xa0]:bytes) -> (eax, edx, cf, pf, af, zf, sf, of); globals=layout=state_00426091")
