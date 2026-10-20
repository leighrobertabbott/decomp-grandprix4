#include <gp4/gp4.h>

struct Unknown_00431838 {
    uint8_t Unknown00[0xe];
    int16_t Unknown0E;
    uint8_t Unknown10[0x8];
    int16_t Unknown18;
    uint8_t Unknown1A[0x2];
    const int16_t* Unknown1C;
    uint8_t Unknown20[0xc5];
    uint8_t Flags0xE5;
};

static inline uint32_t parityFlag8(uint32_t value) {
    uint32_t parity = value & 0xffu;
    parity ^= parity >> 4;
    parity ^= parity >> 2;
    parity ^= parity >> 1;
    return (parity & 1u) == 0u ? 4u : 0u;
}

static inline uint32_t comparisonFlags16(uint32_t left, uint32_t right) {
    const uint32_t result = (left - right) & 0xffffu;
    return (left < right ? 1u : 0u)
        | parityFlag8(result)
        | (((left ^ right ^ result) & 0x10u) != 0u ? 0x10u : 0u)
        | (result == 0u ? 0x40u : 0u)
        | ((result & 0x8000u) != 0u ? 0x80u : 0u)
        | (((left ^ right) & (left ^ result) & 0x8000u) != 0u ? 0x800u : 0u);
}

// FUNCTION: GP4 0x00431838
// CF clear when |body.Unknown0E - (*body.Unknown1C + ax)| exceeds DAT_00603ad0 or, with bit 0 of the
// flag byte set, |Unknown0E - Unknown18| is not below DAT_00603ad2.
void __cdecl FUN_00431838(gp4::Regs* r) {
    const Unknown_00431838* body = reinterpret_cast<const Unknown_00431838*>(static_cast<uintptr_t>(r->esi));
    const int16_t* link = body->Unknown1C;

    const uint32_t sum = static_cast<uint16_t>(*link + static_cast<int16_t>(r->eax));
    uint32_t edx = (reinterpret_cast<uintptr_t>(link) & 0xffff0000u) | sum;

    uint32_t ax = static_cast<uint16_t>(body->Unknown0E - static_cast<int16_t>(sum));
    if ((ax & 0x8000u) != 0u) ax = (0u - ax) & 0xffffu;

    uint32_t flags = comparisonFlags16(ax, static_cast<uint16_t>(GP4_GLOBAL(int16_t, 0x00603ad0)));
    // jg: ZF clear and SF == OF
    const bool greater = (flags & 0x40u) == 0u && (((flags >> 7) ^ (flags >> 11)) & 1u) == 0u;
    if (greater) {
        flags &= ~1u;  // clc
    } else {
        const uint32_t bit = body->Flags0xE5 & 1u;
        flags = parityFlag8(bit) | (bit == 0u ? 0x40u : 0u);
        if (bit == 0u) {
            flags |= 1u;  // stc
        } else {
            ax = static_cast<uint16_t>(body->Unknown0E - body->Unknown18);
            if ((ax & 0x8000u) != 0u) ax = (0u - ax) & 0xffffu;
            flags = comparisonFlags16(ax, static_cast<uint16_t>(GP4_GLOBAL(int16_t, 0x00603ad2)));
            // jl: SF != OF
            const bool less = (((flags >> 7) ^ (flags >> 11)) & 1u) != 0u;
            if (less) flags |= 1u; else flags &= ~1u;
        }
    }

    r->eax = (r->eax & 0xffff0000u) | ax;
    r->edx = edx;
    r->eflags = (r->eflags & ~0x8d5u) | flags;
}
GP4_IMPL(0x00431838, FUN_00431838, "regs(eax:u32, esi:ptr[0xed]:layout=rec_00431838) -> (eax, edx, cf, pf, af, zf, sf, of); globals=layout=state_00431838")
