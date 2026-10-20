#include <gp4/gp4.h>

static inline uint32_t parityFlag8(uint32_t value) {
    uint32_t p = value & 0xffu;
    p ^= p >> 4;
    p ^= p >> 2;
    p ^= p >> 1;
    return (p & 1u) == 0u ? 0x04u : 0u;
}

// Flags of the 32-bit `cmp left, right` (CF PF AF ZF SF OF).
static inline uint32_t cmpFlags32(uint32_t left, uint32_t right) {
    const uint32_t result = left - right;
    return (left < right ? 0x01u : 0u)
        | parityFlag8(result)
        | (((left ^ right ^ result) & 0x10u) != 0u ? 0x10u : 0u)
        | (result == 0u ? 0x40u : 0u)
        | ((result & 0x80000000u) != 0u ? 0x80u : 0u)
        | (((left ^ right) & (left ^ result) & 0x80000000u) != 0u ? 0x800u : 0u);
}

struct Unknown_0042d665_Pair {
    uint8_t Unknown00[0x9a];
    int16_t Unknown9A;
    uint8_t Unknown9C;
    uint8_t Unknown9D[0xe2 - 0x9d];
    uint16_t UnknownE2;
};

struct Unknown_0042d665_Source {
    uint8_t Unknown00[0x10];
    uint8_t Unknown10;
    uint8_t Unknown11[0xa9 - 0x11];
    int8_t UnknownA9;
};

// The word difference of +0xe2 between the two records, or the 0x0040dc8a value when either has
// bit 1 of +0x9c set.
static inline uint16_t recordDelta(const gp4::Regs* r, const Unknown_0042d665_Pair* self, const Unknown_0042d665_Pair* other) {
    if ((self->Unknown9C & 2u) == 0u && (other->Unknown9C & 2u) == 0u) {
        return static_cast<uint16_t>(other->UnknownE2 - self->UnknownE2);
    }
    gp4::Regs call = *r;
    call.st_in = 0;
    call.st_out = 0;
    gp4::call_regs(0x0040dc8a, call);
    return static_cast<uint16_t>(call.eax);
}

// FUNCTION: GP4 0x0042d665
// ecx = byte offset into edi, edi = source record, esi / ebx = the two bodies. CF clear when the
// second weighted comparison fails (returns with CF set otherwise); ebp and edx hold the last
// products, the flags are those of the final 32-bit cmp with CF forced by clc / stc.
void __cdecl FUN_0042d665(gp4::Regs* r) {
    const uint32_t ecxIn = r->ecx;
    Unknown_0042d665_Source* source = reinterpret_cast<Unknown_0042d665_Source*>(static_cast<uintptr_t>(r->edi));
    const Unknown_0042d665_Pair* self = reinterpret_cast<const Unknown_0042d665_Pair*>(static_cast<uintptr_t>(r->esi));
    const Unknown_0042d665_Pair* other = reinterpret_cast<const Unknown_0042d665_Pair*>(static_cast<uintptr_t>(r->ebx));

    uint8_t sign = source->Unknown10;
    if (ecxIn != 0x10u) {
        sign = static_cast<uint8_t>(sign ^ 0x80u);
    }
    GP4_GLOBAL(uint8_t, 0x00604154) = sign;

    uint32_t base = (GP4_FIELD(const uint8_t, source, ecxIn) & 0x7fu) << 6;
    GP4_GLOBAL(uint16_t, 0x00603bf8) = GP4_GLOBAL(uint16_t, 0x00603bfa);

    const uint8_t extra = static_cast<uint8_t>(source->UnknownA9);
    if (extra != 0u && static_cast<int8_t>(static_cast<uint8_t>(extra ^ GP4_GLOBAL(uint8_t, 0x00604154))) >= 0) {
        uint8_t magnitude = extra;
        if (static_cast<int8_t>(magnitude) < 0) {
            magnitude = static_cast<uint8_t>(0u - magnitude);
        }
        int32_t steps = static_cast<int32_t>(magnitude);
        if (steps > 4) {
            steps = 4;
        }
        const int64_t scaled = static_cast<int64_t>(steps << 6) * static_cast<int64_t>(GP4_GLOBAL(int32_t, 0x00603bbc));
        base += static_cast<uint32_t>(static_cast<uint64_t>(scaled) >> 14);
    }
    uint32_t ebp = base;

    uint16_t ax = recordDelta(r, self, other);
    ax = static_cast<uint16_t>(0u - ax);
    int64_t product = static_cast<int64_t>(static_cast<int32_t>(static_cast<int16_t>(ax))) * static_cast<int64_t>(GP4_GLOBAL(int32_t, 0x00603bdc));
    uint32_t eax = static_cast<uint32_t>(static_cast<uint64_t>(product) >> 14);
    uint32_t edx = static_cast<uint32_t>(static_cast<uint64_t>(product) >> 32);
    bool fail = static_cast<int32_t>(eax) >= static_cast<int32_t>(ebp);

    if (!fail) {
        const int32_t selfPos = static_cast<int32_t>(other->Unknown9A);
        const uint32_t weight = ebp - static_cast<uint32_t>(selfPos >> 5);
        uint16_t delta = static_cast<uint16_t>(other->Unknown9A - self->Unknown9A);
        delta = static_cast<uint16_t>(delta + GP4_GLOBAL(uint16_t, 0x00603bf8));
        const int64_t first = static_cast<int64_t>(static_cast<int32_t>(static_cast<int16_t>(delta))) * static_cast<int64_t>(static_cast<int32_t>(weight));
        ebp = static_cast<uint32_t>(first);

        ax = recordDelta(r, self, other);
        ax = static_cast<uint16_t>(0u - ax);
        ax = static_cast<uint16_t>(ax - GP4_GLOBAL(uint16_t, 0x00603b9e));
        const int32_t inner = static_cast<int32_t>(static_cast<int16_t>(ax)) * static_cast<int32_t>(other->Unknown9A);
        product = static_cast<int64_t>(inner) * static_cast<int64_t>(GP4_GLOBAL(int32_t, 0x00603bb4));
        eax = static_cast<uint32_t>(static_cast<uint64_t>(product) >> 14);
        edx = static_cast<uint32_t>(static_cast<uint64_t>(product) >> 32);
        fail = static_cast<int32_t>(eax) >= static_cast<int32_t>(ebp);
    }

    uint32_t flags = cmpFlags32(eax, ebp);
    flags = fail ? (flags | 1u) : (flags & ~1u);
    r->ebp = ebp;
    r->edx = edx;
    r->eflags = (r->eflags & ~0x8d5u) | flags;
}
GP4_IMPL(0x0042d665, FUN_0042d665, "regs(ecx:i32[8..24], edi:ptr[0xb1]:layout=seg_0042d665, esi:ptr[0x225]:bytes, ebx:ptr[0x225]:bytes) -> (edx, ebp, cf, pf, af, zf, sf, of); globals=fuzz:i32")
