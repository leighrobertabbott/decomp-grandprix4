#include <gp4/gp4.h>

static inline uint32_t parityFlag(uint32_t value) {
    value &= 0xffu;
    value ^= value >> 4;
    value ^= value >> 2;
    value ^= value >> 1;
    return (value & 1u) == 0u ? 4u : 0u;
}

// Flags of a 32-bit logic result (test): CF = OF = AF = 0.
static inline uint32_t logicFlags(uint32_t result) {
    return parityFlag(result) | (result == 0u ? 0x40u : 0u) | ((result & 0x80000000u) != 0u ? 0x80u : 0u);
}

// Flags of the 32-bit `left - right` (sub / cmp), CF included.
static inline uint32_t subFlags(uint32_t left, uint32_t right) {
    const uint32_t result = left - right;
    return parityFlag(result)
        | (left < right ? 0x01u : 0u)
        | (((left ^ right ^ result) & 0x10u) != 0u ? 0x10u : 0u)
        | (result == 0u ? 0x40u : 0u)
        | ((result & 0x80000000u) != 0u ? 0x80u : 0u)
        | ((((left ^ right) & (left ^ result)) & 0x80000000u) != 0u ? 0x800u : 0u);
}

// edi = a track-segment record whose dword +0x44 links to another record R (0 = none). R's signed
// bytes +0x7c / +0x7d are a first / last record offset; when last >= first the walk of 0x00405d65 is
// started at R + first*0x160 for the point at esi, and CF = (end of walk >= [0x00891500]).
// FUNCTION: GP4 0x0042b3c8
void __cdecl FUN_0042b3c8(gp4::Regs* r) {
    const uint32_t link = GP4_FIELD(uint32_t, reinterpret_cast<void*>(static_cast<uintptr_t>(r->edi)), 0x44);
    uint32_t flags;

    if (link == 0u) {
        // test dword [edi+0x44], -1 ; je -> clc
        flags = logicFlags(link);
    } else {
        void* record = reinterpret_cast<void*>(static_cast<uintptr_t>(link));
        const int32_t first = static_cast<int32_t>(GP4_FIELD(int8_t, record, 0x7c));
        const int32_t last = static_cast<int32_t>(GP4_FIELD(int8_t, record, 0x7d));
        const uint32_t span = static_cast<uint32_t>(last) - static_cast<uint32_t>(first);
        r->edi = link;
        r->eax = static_cast<uint32_t>(first);
        r->ecx = span;
        flags = subFlags(static_cast<uint32_t>(last), static_cast<uint32_t>(first)) & ~1u;
        if (static_cast<int32_t>(span) >= 0) {
            gp4::Regs call = *r;
            call.eax = static_cast<uint32_t>(first * 0x160);
            call.edi = link + call.eax;
            gp4::call_regs(0x00405d65, call);
            *r = call;
            const uint32_t limit = GP4_GLOBAL(uint32_t, 0x00891500);
            // cmp edi, [limit] ; jb -> clc ; else stc
            flags = (subFlags(call.edi, limit) & ~1u) | (call.edi >= limit ? 1u : 0u);
        }
    }

    r->eflags = (r->eflags & ~0x8d5u) | flags;
}
GP4_IMPL(0x0042b3c8, FUN_0042b3c8, "regs(esi:ptr[0x40]:layout=pos_00405cf8, edi:rec[0x8943a0,0x160,0..25]) -> (eax, ecx, edx, edi, cf, pf, af, zf, sf, of); globals=layout=ring_00405a12+links_0042b3c8")
