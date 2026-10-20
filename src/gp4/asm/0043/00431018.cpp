#include <gp4/gp4.h>

static inline uint32_t parityFlag(uint32_t value) {
    uint32_t parity = value & 0xffu;
    parity ^= parity >> 4;
    parity ^= parity >> 2;
    parity ^= parity >> 1;
    return (parity & 1u) == 0u ? 4u : 0u;
}

// Arithmetic flags (CF PF AF ZF SF OF) of `left - right` at the given width (8 or 16 bits).
static inline uint32_t subFlags(uint32_t left, uint32_t right, uint32_t bits) {
    const uint32_t mask = (1u << bits) - 1u;
    const uint32_t sign = 1u << (bits - 1u);
    left &= mask;
    right &= mask;
    const uint32_t result = (left - right) & mask;
    return (left < right ? 1u : 0u)
        | parityFlag(result)
        | (((left ^ right ^ result) & 0x10u) != 0u ? 0x10u : 0u)
        | (result == 0u ? 0x40u : 0u)
        | ((result & sign) != 0u ? 0x80u : 0u)
        | ((((left ^ right) & (left ^ result) & sign) != 0u) ? 0x800u : 0u);
}

// Flags of a logic result (test / or) at the given width: CF = OF = AF = 0.
static inline uint32_t logicFlags(uint32_t result, uint32_t bits) {
    const uint32_t mask = (1u << bits) - 1u;
    const uint32_t sign = 1u << (bits - 1u);
    result &= mask;
    return parityFlag(result) | (result == 0u ? 0x40u : 0u) | ((result & sign) != 0u ? 0x80u : 0u);
}

// Car record fields touched here (two records: esi and ebx).
struct Unknown_00431018 {
    uint8_t Unknown00[0x8d];
    uint8_t Flags8D;
    uint8_t Unknown8E[0x90 - 0x8e];
    int16_t Word90;
    uint8_t Unknown92[0x9c - 0x92];
    uint8_t Flags9C;
    uint8_t Unknown9D[0xe2 - 0x9d];
    int16_t WordE2;
    uint8_t UnknownE4[0x12c - 0xe4];
    uint32_t Nearest12C;        // record that set the +0x138 gap
    uint32_t Nearest130;        // record that set the +0x13a gap
    uint8_t Flags134;           // bit 7: +0x138 valid, bit 6: +0x13a valid
    uint8_t Unknown135[0x138 - 0x135];
    uint16_t Gap138;
    uint16_t Gap13A;
    uint8_t Unknown13C[0x186 - 0x13c];
    int16_t Word186;
};

// FUNCTION: GP4 0x00431018
// Compares two car records (esi, ebx): the signed 16-bit difference of their positions (+0x90, or
// +0x186 when exactly one record has +0x8d bit 1 and that record has +0x9c bit 4) is rejected when it
// is within the first limit [0x603b22] but the lateral difference (+0xe2, or the 0x0040dc8a result when
// either +0x9c has bit 1) exceeds [0x603b24]. Otherwise the smaller absolute gap is recorded in each
// record's +0x138 (behind) / +0x13a (ahead) with the other record in +0x12c / +0x130 and flag
// bits 7 / 6 in +0x134. Outputs: edx (low word as the original leaves it) and the flags.
void __cdecl FUN_00431018(gp4::Regs* r) {
    Unknown_00431018* esi = reinterpret_cast<Unknown_00431018*>(static_cast<uintptr_t>(r->esi));
    Unknown_00431018* ebx = reinterpret_cast<Unknown_00431018*>(static_cast<uintptr_t>(r->ebx));
    uint32_t edx = r->edx;
    uint32_t flags = 0;

    const bool esiBit = (esi->Flags8D & 2u) != 0u;
    const bool ebxBit = (ebx->Flags8D & 2u) != 0u;
    uint16_t diff;
    if (esiBit == ebxBit) {
        diff = static_cast<uint16_t>(ebx->Word90 - esi->Word90);
    } else {
        const uint8_t gate = esiBit ? esi->Flags9C : ebx->Flags9C;
        if ((gate & 0x10u) == 0u) {
            r->eflags = (r->eflags & ~0x8d5u) | logicFlags(gate & 0x10u, 8u);
            return;
        }
        diff = static_cast<uint16_t>(ebx->Word186 - esi->Word186);
    }

    const uint32_t pushed = (edx & 0xffff0000u) | diff;

    uint16_t dx = diff;
    if (static_cast<int16_t>(dx) < 0) {
        dx = static_cast<uint16_t>(0u - dx);
    }
    const int16_t nearLimit = GP4_GLOBAL(int16_t, 0x00603b22);
    if (static_cast<int16_t>(dx) < nearLimit) {
        uint16_t lateral;
        if (((esi->Flags9C | ebx->Flags9C) & 2u) == 0u) {
            lateral = static_cast<uint16_t>(ebx->WordE2 - esi->WordE2);
            flags = subFlags(static_cast<uint16_t>(ebx->WordE2), static_cast<uint16_t>(esi->WordE2), 16u);
        } else {
            gp4::Regs call = *r;
            call.st_in = 0;
            call.st_out = 0;
            gp4::call_regs(0x0040dc8a, call);
            lateral = static_cast<uint16_t>(call.eax);
            flags = call.eflags & 0x8d5u;
        }
        if ((flags & 0x80u) != 0u) {
            lateral = static_cast<uint16_t>(0u - lateral);
        }
        const int16_t lateralLimit = GP4_GLOBAL(int16_t, 0x00603b24);
        flags = subFlags(lateral, static_cast<uint16_t>(lateralLimit), 16u);
        if (static_cast<int16_t>(lateral) > lateralLimit) {
            r->edx = pushed;
            r->eflags = (r->eflags & ~0x8d5u) | flags;
            return;
        }
    }

    uint16_t gap = diff;
    if (static_cast<int16_t>(gap) < 0) {
        gap = static_cast<uint16_t>(0u - gap);
        if ((esi->Flags134 & 0x80u) == 0u || gap <= esi->Gap138) {
            esi->Gap138 = gap;
            esi->Nearest12C = static_cast<uint32_t>(reinterpret_cast<uintptr_t>(ebx));
            esi->Flags134 = static_cast<uint8_t>(esi->Flags134 | 0x80u);
        }
        if ((ebx->Flags134 & 0x40u) == 0u || gap <= ebx->Gap13A) {
            ebx->Gap13A = gap;
            ebx->Nearest130 = static_cast<uint32_t>(reinterpret_cast<uintptr_t>(esi));
            ebx->Flags134 = static_cast<uint8_t>(ebx->Flags134 | 0x40u);
            flags = logicFlags(ebx->Flags134, 8u);
        } else {
            flags = subFlags(gap, ebx->Gap13A, 16u);
        }
    } else {
        if ((esi->Flags134 & 0x40u) == 0u || gap <= esi->Gap13A) {
            esi->Gap13A = gap;
            esi->Nearest130 = static_cast<uint32_t>(reinterpret_cast<uintptr_t>(ebx));
            esi->Flags134 = static_cast<uint8_t>(esi->Flags134 | 0x40u);
        }
        if ((ebx->Flags134 & 0x80u) == 0u || gap <= ebx->Gap138) {
            ebx->Gap138 = gap;
            ebx->Nearest12C = static_cast<uint32_t>(reinterpret_cast<uintptr_t>(esi));
            ebx->Flags134 = static_cast<uint8_t>(ebx->Flags134 | 0x80u);
            flags = logicFlags(ebx->Flags134, 8u);
        } else {
            flags = subFlags(gap, ebx->Gap138, 16u);
        }
    }

    r->edx = (pushed & 0xffff0000u) | gap;
    r->eflags = (r->eflags & ~0x8d5u) | flags;
}
GP4_IMPL(0x00431018, FUN_00431018, "regs(edx:u32, ebx:ptr[0x225]:bytes, esi:ptr[0x225]:bytes) -> (edx, cf, pf, af, zf, sf, of); globals=fuzz:i32")
