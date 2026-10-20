#include <gp4/gp4.h>

static inline uint32_t parityFlag(uint32_t value) {
    value &= 0xffu;
    value ^= value >> 4;
    value ^= value >> 2;
    value ^= value >> 1;
    return (value & 1u) == 0u ? 4u : 0u;
}

// Flags of a 32-bit `cmp left, right` (CF PF AF ZF SF OF).
static inline uint32_t cmpFlags32(uint32_t left, uint32_t right) {
    const uint32_t result = left - right;
    return (left < right ? 1u : 0u)
        | parityFlag(result)
        | (((left ^ right ^ result) & 0x10u) != 0u ? 0x10u : 0u)
        | (result == 0u ? 0x40u : 0u)
        | ((result & 0x80000000u) != 0u ? 0x80u : 0u)
        | ((((left ^ right) & (left ^ result)) & 0x80000000u) != 0u ? 0x800u : 0u);
}

struct Unknown_0043c677_Record {
    uint8_t Unknown00[0x1c];
    uint32_t Unknown1C;
    uint8_t Unknown20[0x230 - 0x20];
    uint8_t Flags230;
};

struct Unknown_0043c677_Segment {
    uint8_t Unknown00[0x7a];
    uint16_t Unknown7A;
};

// Fraction of the way from [0x00891500] to [0x00866f08] covered by the position [esi+0x1c], scaled by
// [0x0062d250] and by the table factor 0x4000 + 16 * table[10 * word +0x7a of [0x00866ed0] + 1] (both
// in Q14). Returns [0x0062d250] below [0x00891500] and 0 when bit 7 of +0x230 is set or the position
// is at or beyond [0x00866f08]. ebx is saved and restored.
// FUNCTION: GP4 0x0043c677
void __cdecl FUN_0043c677(gp4::Regs* r) {
    const Unknown_0043c677_Record* record = reinterpret_cast<const Unknown_0043c677_Record*>(static_cast<uintptr_t>(r->esi));

    uint32_t eax = GP4_GLOBAL(uint32_t, 0x0062d250);
    const uint32_t edi = record->Unknown1C;

    const uint32_t lower = GP4_GLOBAL(uint32_t, 0x00891500);
    if (edi < lower) {
        r->eax = eax;
        r->edi = edi;
        r->eflags = (r->eflags & ~0x8d5u) | cmpFlags32(edi, lower);
        return;
    }

    if ((record->Flags230 & 0x80u) != 0u) {
        // test byte [esi+0x230], 0x80: result 0x80 -> SF only
        r->eax = 0u;
        r->edi = edi;
        r->eflags = (r->eflags & ~0x8d5u) | 0x80u;
        return;
    }

    const uint32_t upper = GP4_GLOBAL(uint32_t, 0x00866f08);
    if (edi >= upper) {
        r->eax = 0u;
        r->edi = edi;
        r->eflags = (r->eflags & ~0x8d5u) | cmpFlags32(edi, upper);
        return;
    }

    // neg edi ; add edi, [0x866f08] ; mov eax, edi ; cdq ; shld edx, eax, 14 ; shl eax, 14
    const int32_t distance = static_cast<int32_t>(upper - edi);
    const int64_t dividend = static_cast<int64_t>(distance) * 16384;
    const int32_t span = static_cast<int32_t>(upper - GP4_GLOBAL(uint32_t, 0x00891500));
    int32_t remainder;
    const int32_t quotient = gp4::idiv64(dividend, span, &remainder);

    // imul dword [0x62d250] ; shrd eax, edx, 14
    const int64_t scaled = static_cast<int64_t>(quotient) * GP4_GLOBAL(int32_t, 0x0062d250);
    const int32_t first = static_cast<int32_t>(static_cast<uint32_t>(static_cast<uint64_t>(scaled) >> 14));

    const Unknown_0043c677_Segment* segment = reinterpret_cast<const Unknown_0043c677_Segment*>(static_cast<uintptr_t>(GP4_GLOBAL(uint32_t, 0x00866ed0)));
    const uint32_t index = static_cast<uint32_t>(segment->Unknown7A) * 10u;
    const uint32_t entry = GP4_ARRAY(const uint8_t, 0x00675a83)[index];
    const int32_t factor = static_cast<int32_t>((entry << 4) + 0x4000u);

    // imul ebx ; shrd eax, edx, 14
    const int64_t product = static_cast<int64_t>(first) * factor;
    const uint32_t result = static_cast<uint32_t>(static_cast<uint64_t>(product) >> 14);
    const uint32_t previous = static_cast<uint32_t>(static_cast<uint64_t>(product) >> 13);

    // flags of the final SHRD by 14 (pinned oracle model: AF = 0, CF = last bit out,
    // OF = bit31(value shifted by 13 ^ result))
    const uint32_t flags = (previous & 1u)
        | parityFlag(result)
        | (result == 0u ? 0x40u : 0u)
        | ((result >> 24) & 0x80u)
        | ((((previous ^ result) >> 31) & 1u) != 0u ? 0x800u : 0u);

    r->eax = result;
    r->edx = static_cast<uint32_t>(static_cast<uint64_t>(product) >> 32);
    r->edi = static_cast<uint32_t>(span);
    r->eflags = (r->eflags & ~0x8d5u) | flags;
}
GP4_IMPL(0x0043c677, FUN_0043c677, "regs(esi:ptr[0x238]:layout=car_0043c677) -> (eax, edx, edi, cf, pf, af, zf, sf, of); globals=layout=state_0043c677")
