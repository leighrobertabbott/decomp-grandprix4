#include <gp4/gp4.h>

static inline uint32_t parityFlag(uint32_t value) {
    value &= 0xffu;
    value ^= value >> 4;
    value ^= value >> 2;
    value ^= value >> 1;
    return (value & 1u) == 0u ? 4u : 0u;
}

// Arithmetic flags of `cmp left, right`.
static inline uint32_t subFlags(uint32_t left, uint32_t right) {
    const uint32_t result = left - right;
    return (left < right ? 1u : 0u)
        | parityFlag(result)
        | (((left ^ right ^ result) & 0x10u) != 0u ? 0x10u : 0u)
        | (result == 0u ? 0x40u : 0u)
        | ((result & 0x80000000u) != 0u ? 0x80u : 0u)
        | (((left ^ right) & (left ^ result) & 0x80000000u) != 0u ? 0x800u : 0u);
}

// pushal; call 0x00432372; [esi+0x1d8] += eax; the larger (unsigned) of [esi+0x1d8] and
// [esi+0x1dc] is stored to [esi+0xf0]; popal. The registers all come back; the flags are those
// of `cmp eax, [esi+0x1dc]`.
// FUNCTION: GP4 0x0043240e
void __cdecl FUN_0043240e(gp4::Regs* r) {
    gp4::Regs inner = *r;
    gp4::call_regs(0x00432372, inner);

    uint8_t* record = reinterpret_cast<uint8_t*>(static_cast<uintptr_t>(inner.esi));
    const uint32_t total = GP4_FIELD(uint32_t, record, 0x1d8) + inner.eax;
    GP4_FIELD(uint32_t, record, 0x1d8) = total;
    const uint32_t limit = GP4_FIELD(uint32_t, record, 0x1dc);
    GP4_FIELD(uint32_t, record, 0xf0) = total >= limit ? total : limit;

    r->eflags = (r->eflags & ~0x8d5u) | subFlags(total, limit);
}
GP4_IMPL(0x0043240e, FUN_0043240e, "regs(eax:i32, esi:ptr[0x1e8]:bytes) -> (cf, pf, af, zf, sf, of); globals=layout=state_00432372")
