#include <gp4/gp4.h>

static inline uint32_t parityFlag(uint32_t value) {
    value &= 0xffu;
    value ^= value >> 4;
    value ^= value >> 2;
    value ^= value >> 1;
    return (value & 1u) == 0u ? 4u : 0u;
}

// flags of "add left, right"
static inline uint32_t addFlags(uint32_t left, uint32_t right) {
    const uint32_t result = left + right;
    return (result < left ? 1u : 0u)
        | parityFlag(result)
        | (((left ^ right ^ result) & 0x10u) != 0u ? 0x10u : 0u)
        | (result == 0u ? 0x40u : 0u)
        | ((result & 0x80000000u) != 0u ? 0x80u : 0u)
        | ((~(left ^ right) & (left ^ result) & 0x80000000u) != 0u ? 0x800u : 0u);
}

// flags of "cmp left, right"
static inline uint32_t subFlags(uint32_t left, uint32_t right) {
    const uint32_t result = left - right;
    return (left < right ? 1u : 0u)
        | parityFlag(result)
        | (((left ^ right ^ result) & 0x10u) != 0u ? 0x10u : 0u)
        | (result == 0u ? 0x40u : 0u)
        | ((result & 0x80000000u) != 0u ? 0x80u : 0u)
        | (((left ^ right) & (left ^ result) & 0x80000000u) != 0u ? 0x800u : 0u);
}

// flags of a byte "or"/"and"/"test" result (CF = OF = AF = 0)
static inline uint32_t logicFlags8(uint32_t value) {
    value &= 0xffu;
    return parityFlag(value) | (value == 0u ? 0x40u : 0u) | (value & 0x80u);
}

// FUNCTION: GP4 0x004163a7
// esi = record with a small state machine: state = sbyte [esi+0x237], deadline = dword [esi+0x1e8],
// clock = dword [0x66d814]. state < 2: deadline = clock + [0x62693c], state = 2 (eax = [0x62693c]).
// state == 2 and a nonzero deadline that has been reached (clock - deadline >= 0): deadline =
// clock + [0x626940] (eax = [0x626940]), set bit 1 of +0x7b, clear bit 0 of +0xce, state = 3, and
// when edi >= [0x891500] set bit 5 of +0xfe. state > 2 and a reached nonzero deadline: state = 0.
// cl receives the state byte; eax is otherwise preserved (push/pop). The flags are those of the
// last flag writer on the taken path.
void __cdecl FUN_004163a7(gp4::Regs* r) {
    uint8_t* const rec = reinterpret_cast<uint8_t*>(static_cast<uintptr_t>(r->esi));
    const uint32_t edi = r->edi;

    const int8_t state = GP4_FIELD(int8_t, rec, 0x237);
    r->ecx = (r->ecx & 0xffffff00u) | static_cast<uint8_t>(state);

    const uint32_t clock = GP4_GLOBAL(uint32_t, 0x0066d814);

    if (state < 2) {
        const uint32_t delta = GP4_GLOBAL(uint32_t, 0x0062693c);
        r->eax = delta;
        GP4_FIELD(uint32_t, rec, 0x1e8) = clock;
        GP4_FIELD(uint32_t, rec, 0x1e8) += delta;
        GP4_FIELD(uint8_t, rec, 0x237) = 2;
        r->eflags = (r->eflags & ~0x8d5u) | addFlags(clock, delta);
        return;
    }

    const uint32_t deadline = GP4_FIELD(uint32_t, rec, 0x1e8);
    if (deadline == 0u) {
        // test dword [esi+0x1e8], -1 on zero: ZF and PF set
        r->eflags = (r->eflags & ~0x8d5u) | 0x44u;
        return;
    }

    // neg eax; add eax, [0x66d814]: clock - deadline, flags of the add
    const uint32_t negated = 0u - deadline;
    const uint32_t elapsed = negated + clock;
    const uint32_t checkFlags = addFlags(negated, clock);
    if ((elapsed & 0x80000000u) != 0u) {
        r->eflags = (r->eflags & ~0x8d5u) | checkFlags;
        return;
    }

    if (state == 2) {
        const uint32_t delta = GP4_GLOBAL(uint32_t, 0x00626940);
        r->eax = delta;
        GP4_FIELD(uint32_t, rec, 0x1e8) = clock;
        GP4_FIELD(uint32_t, rec, 0x1e8) += delta;
        GP4_FIELD(uint8_t, rec, 0x7b) |= 0x02u;
        GP4_FIELD(uint8_t, rec, 0xce) &= 0xfeu;
        GP4_FIELD(uint8_t, rec, 0x237) = 3;
        const uint32_t limit = GP4_GLOBAL(uint32_t, 0x00891500);
        if (edi < limit) {
            r->eflags = (r->eflags & ~0x8d5u) | subFlags(edi, limit);
        } else {
            GP4_FIELD(uint8_t, rec, 0xfe) |= 0x20u;
            r->eflags = (r->eflags & ~0x8d5u) | logicFlags8(GP4_FIELD(uint8_t, rec, 0xfe));
        }
        return;
    }

    GP4_FIELD(uint8_t, rec, 0x237) = 0;
    r->eflags = (r->eflags & ~0x8d5u) | checkFlags;
}
GP4_IMPL(0x004163a7, FUN_004163a7, "regs(ecx:u32, edi:u32, esi:ptr[0x23f]:bytes) -> (eax, ecx, cf, pf, af, zf, sf, of); globals=fuzz:i32")
