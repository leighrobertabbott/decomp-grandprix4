#include <gp4/gp4.h>

// Only the fields this routine reads.
struct Unknown_00406b0a {
    uint8_t Unknown00[0x44];
    uint32_t Unknown44;
    uint8_t Unknown48[0x34];
    int8_t Unknown7C;
    int8_t Unknown7D;
};
static_assert(sizeof(Unknown_00406b0a) == 0x80, "layout");

static inline uint32_t parityFlag(uint32_t value) {
    value &= 0xffu;
    value ^= value >> 4;
    value ^= value >> 2;
    value ^= value >> 1;
    return (value & 1u) == 0u ? 4u : 0u;
}

// Flags of the 32-bit `left - right`.
static inline uint32_t subtractFlags32(uint32_t left, uint32_t right) {
    const uint32_t result = left - right;
    return (left < right ? 1u : 0u)
        | parityFlag(result)
        | (((left ^ right ^ result) & 0x10u) != 0u ? 0x10u : 0u)
        | (result == 0u ? 0x40u : 0u)
        | ((result & 0x80000000u) != 0u ? 0x80u : 0u)
        | ((((left ^ right) & (left ^ result)) & 0x80000000u) != 0u ? 0x800u : 0u);
}

// Flags of `or value, value` (CF = OF = AF = 0).
static inline uint32_t orSelfFlags32(uint32_t value) {
    return parityFlag(value) | (value == 0u ? 0x40u : 0u) | ((value & 0x80000000u) != 0u ? 0x80u : 0u);
}

// Flags of `dec value` (CF untouched, so left clear here).
static inline uint32_t decFlags32(uint32_t value) {
    const uint32_t result = value - 1u;
    return parityFlag(result)
        | ((value & 0xfu) == 0u ? 0x10u : 0u)
        | (result == 0u ? 0x40u : 0u)
        | ((result & 0x80000000u) != 0u ? 0x80u : 0u)
        | (value == 0x80000000u ? 0x800u : 0u);
}

// edi = a ring record (below DAT_00891500). Scans the records from +0x44 + (signed byte +0x7c)*0x160
// (count = byte +0x7d - byte +0x7c + 1) for the first one for which 0x406303 sets CF, stopping
// below DAT_00866ef8. A hit stores the record in DAT_00629f10 and clears DAT_00629f08 and returns
// CF=1; otherwise CF=0. A pushal ... popal wrapper: every register is restored and the flags are
// those of the last flag writer with CF forced by clc / stc.
// FUNCTION: GP4 0x00406b0a
void __cdecl FUN_00406b0a(gp4::Regs* r) {
    const uint32_t start = r->edi;
    const uint32_t limit = GP4_GLOBAL(uint32_t, 0x00891500);
    uint32_t flags = subtractFlags32(start, limit);
    bool found = false;

    if (start < limit) {
        const Unknown_00406b0a* record = reinterpret_cast<const Unknown_00406b0a*>(static_cast<uintptr_t>(start));
        const int32_t first = record->Unknown7C;
        int32_t remaining = static_cast<int32_t>(record->Unknown7D) - first;
        flags = subtractFlags32(static_cast<uint32_t>(static_cast<int32_t>(record->Unknown7D)), static_cast<uint32_t>(first));
        if (remaining >= 0) {
            const uint32_t link = record->Unknown44;
            flags = orSelfFlags32(link);
            if (link != 0u) {
                uint32_t current = static_cast<uint32_t>(first) * 0x160u + link;
                for (;;) {
                    const uint32_t next = current + 0x160u;
                    flags = subtractFlags32(next, GP4_GLOBAL(uint32_t, 0x00866ef8));
                    if (next >= GP4_GLOBAL(uint32_t, 0x00866ef8)) break;

                    gp4::Regs s = *r;
                    s.edi = current;
                    s.ebx = next;
                    gp4::call_regs(0x00406303, s);
                    flags = s.eflags & 0x8d5u;
                    if ((s.eflags & 1u) != 0u) {
                        GP4_GLOBAL(uint32_t, 0x00629f10) = current;
                        GP4_GLOBAL(uint32_t, 0x00629f08) = 0u;
                        found = true;
                        break;
                    }
                    current += 0x160u;
                    flags = decFlags32(static_cast<uint32_t>(remaining));
                    remaining -= 1;
                    if (remaining < 0) break;
                }
            }
        }
    }

    r->eflags = (r->eflags & ~0x8d5u) | ((flags & 0x8d4u) | (found ? 1u : 0u));
}
GP4_IMPL(0x00406b0a, FUN_00406b0a, "regs(edi:ptr[0x85]:layout=rec_00406b0a) -> (cf, pf, af, zf, sf, of); globals=layout=state_00406b0a")
