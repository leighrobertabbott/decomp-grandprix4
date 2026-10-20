#include <gp4/gp4.h>

static inline uint32_t parityFlag8(uint32_t value) {
    uint32_t parity = value & 0xffu;
    parity ^= parity >> 4;
    parity ^= parity >> 2;
    parity ^= parity >> 1;
    return (parity & 1u) == 0u ? 4u : 0u;
}

static inline uint32_t logicFlags32(uint32_t result) {
    return parityFlag8(result)
        | (result == 0u ? 0x40u : 0u)
        | ((result & 0x80000000u) != 0u ? 0x80u : 0u);
}

static inline uint32_t subtractionFlags32(uint32_t left, uint32_t right) {
    const uint32_t result = left - right;
    return (left < right ? 1u : 0u)
        | logicFlags32(result)
        | (((left ^ right ^ result) & 0x10u) != 0u ? 0x10u : 0u)
        | (((left ^ right) & (left ^ result) & 0x80000000u) != 0u ? 0x800u : 0u);
}

// FUNCTION: GP4 0x0040e802
// Returns eax = a per-record value (dword +0x274, or global 0x66d814 minus it) with CF = 1 when the
// record qualifies (flag 0x80 in byte +0x26b and mode byte +0xe4 is not 1); otherwise eax = 0, CF = 0.
void __cdecl FUN_0040e802(gp4::Regs* r) {
    uint8_t* record = reinterpret_cast<uint8_t*>(static_cast<uintptr_t>(r->esi));
    const uint32_t kClearedFlags = ~0x8d5u;

    uint32_t flags;        // flags before the final stc / clc
    uint32_t value = 0u;
    bool qualifies = false;

    if ((GP4_FIELD(uint8_t, record, 0x26b) & 0x80u) == 0u) {
        // test byte [esi+0x26b], 0x80 -> zero
        flags = logicFlags32(0u);
    } else {
        const int8_t mode = GP4_FIELD(int8_t, record, 0xe4);
        if (mode == 1) {
            // cmp byte [esi+0xe4], 1 -> equal
            flags = logicFlags32(0u);
        } else if (mode >= 2 && mode <= 5) {
            // mov eax,[0x66d814]; sub eax,[esi+0x274]
            const uint32_t minuend = GP4_GLOBAL(uint32_t, 0x0066d814);
            const uint32_t subtrahend = GP4_FIELD(uint32_t, record, 0x274);
            value = minuend - subtrahend;
            flags = subtractionFlags32(minuend, subtrahend);
            qualifies = true;
        } else {
            const uint32_t gate = GP4_FIELD(uint32_t, record, 0x1fc);
            flags = logicFlags32(gate);
            if (gate != 0u) {
                value = GP4_FIELD(uint32_t, record, 0x274);
                qualifies = true;
            }
        }
    }

    // stc / clc: only CF changes
    flags = qualifies ? (flags | 1u) : (flags & ~1u);
    r->eax = value;
    r->eflags = (r->eflags & kClearedFlags) | flags;
}
GP4_IMPL(0x0040e802, FUN_0040e802, "regs(esi:ptr[0x27c]:bytes) -> (eax, cf, pf, af, zf, sf, of); globals=fuzz:i32")
