#include <gp4/gp4.h>

static inline uint32_t subFlags32(uint32_t left, uint32_t right) {
    const uint32_t result = left - right;
    uint32_t parity = result & 0xffu;
    parity ^= parity >> 4;
    parity ^= parity >> 2;
    parity ^= parity >> 1;
    return (left < right ? 1u : 0u)
        | ((parity & 1u) == 0u ? 4u : 0u)
        | (((left ^ right ^ result) & 0x10u) != 0u ? 0x10u : 0u)
        | (result == 0u ? 0x40u : 0u)
        | ((result & 0x80000000u) != 0u ? 0x80u : 0u)
        | (((left ^ right) & (left ^ result) & 0x80000000u) != 0u ? 0x800u : 0u);
}

// FUNCTION: GP4 0x004062b1
// Signed distance eax - edx between two positions on a wrapping track. A value below
// DAT_00891500 is dereferenced as a record (its dword +0x44) when the other operand is not below it.
// The same-side case wraps the difference by DAT_00866ec4 when it reaches DAT_00866ed8
// (the order is swapped and the result negated when eax < edx). edx is preserved; the flags are
// those of the last arithmetic instruction (sub, cmp or neg) on each path.
void __cdecl FUN_004062b1(gp4::Regs* r) {
    const uint32_t position = r->eax;
    uint32_t other = r->edx;
    const uint32_t boundary = GP4_GLOBAL(uint32_t, 0x00891500);

    uint32_t result;
    uint32_t flags;
    if (position >= boundary) {
        if (other < boundary) {
            other = GP4_FIELD(uint32_t, reinterpret_cast<uint8_t*>(static_cast<uintptr_t>(other)), 0x44);
        }
        result = position - other;                      // sub eax, edx
        flags = subFlags32(position, other);
    } else {
        if (other >= boundary) {
            other = GP4_FIELD(uint32_t, reinterpret_cast<uint8_t*>(static_cast<uintptr_t>(other)), 0x44);
        }
        if (position < other) {
            // xchg edx, eax / sub eax, edx
            uint32_t span = other - position;
            if (static_cast<int32_t>(span) >= static_cast<int32_t>(GP4_GLOBAL(uint32_t, 0x00866ed8))) {
                span -= GP4_GLOBAL(uint32_t, 0x00866ec4);
            }
            result = 0u - span;                         // neg eax
            flags = subFlags32(0u, span);
        } else {
            uint32_t span = position - other;
            const uint32_t limit = GP4_GLOBAL(uint32_t, 0x00866ed8);
            flags = subFlags32(span, limit);            // cmp eax, [0x866ed8]
            if (static_cast<int32_t>(span) >= static_cast<int32_t>(limit)) {
                const uint32_t wrap = GP4_GLOBAL(uint32_t, 0x00866ec4);
                flags = subFlags32(span, wrap);         // sub eax, [0x866ec4]
                span -= wrap;
            }
            result = span;
        }
    }

    r->eax = result;
    r->eflags = (r->eflags & ~0x8d5u) | flags;
}
GP4_IMPL(0x004062b1, FUN_004062b1, "regs(eax:u32, edx:ptr[0x48]:bytes) -> (eax, cf, pf, af, zf, sf, of); globals=fuzz:i32")
