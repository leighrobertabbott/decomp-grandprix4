#include <gp4/gp4.h>

struct Unknown_00427cd1_Segment {
    uint8_t Unknown00[0x52];
    int16_t Word52;
    int16_t Word54;
    uint8_t Unknown56[0xc8 - 0x56];
    int16_t WordC8;
};

struct Unknown_00427cd1_Car {
    uint8_t Unknown00[0x71];
    uint8_t Flags71;
    uint8_t Unknown72[0x90 - 0x72];
    int16_t Word90;
    uint8_t Unknown92[0x9c - 0x92];
    uint8_t Flags9C;
    uint8_t Unknown9D[0xfe - 0x9d];
    uint8_t FlagsFE;
    uint8_t UnknownFF[0x122 - 0xff];
    uint8_t Flags122;
    uint8_t Unknown123[0x163 - 0x123];
    uint8_t Flags163;
};

static inline uint32_t parityFlag8(uint32_t value) {
    uint32_t p = value & 0xffu;
    p ^= p >> 4;
    p ^= p >> 2;
    p ^= p >> 1;
    return (p & 1u) == 0u ? 0x04u : 0u;
}

// Flags of "cmp left, right" on a dword.
static inline uint32_t cmpFlags32(uint32_t left, uint32_t right) {
    const uint32_t result = left - right;
    return (left < right ? 0x001u : 0u)
        | parityFlag8(result)
        | (((left ^ right ^ result) & 0x10u) != 0u ? 0x010u : 0u)
        | (result == 0u ? 0x040u : 0u)
        | ((result & 0x80000000u) != 0u ? 0x080u : 0u)
        | ((((left ^ right) & (left ^ result)) & 0x80000000u) != 0u ? 0x800u : 0u);
}

// Flags of "add left, right" on a dword.
static inline uint32_t addFlags32(uint32_t left, uint32_t right) {
    const uint32_t result = left + right;
    return (result < left ? 0x001u : 0u)
        | parityFlag8(result)
        | (((left ^ right ^ result) & 0x10u) != 0u ? 0x010u : 0u)
        | (result == 0u ? 0x040u : 0u)
        | ((result & 0x80000000u) != 0u ? 0x080u : 0u)
        | ((((left ^ result) & (right ^ result)) & 0x80000000u) != 0u ? 0x800u : 0u);
}

// FUNCTION: GP4 0x00427cd1
// Blends two lane-edge words of the segment record in edi (esi = the car): the word +0xc8 (ecx) and a
// bound, +0x52 when bit 3 of the car's +0x71 is set, else +0x54. Bit 6 of car byte +0xfe halves the
// distance to the bound; bit 7 of +0x163 without bit 7 of +0x122 (or bit 4 of +0x9c) hands the word
// +0x90 of the car to 0x00427bc1, while bit 7 of +0x122 mirrors the value about the bound. Otherwise,
// for a segment before the pit start [0x00891500], the value is scaled towards the bound by the Q14
// factor [0x00624858]. eax is the result; ecx and edx are restored; flags are those of the last
// compare/add (or of the callee).
void __cdecl FUN_00427cd1(gp4::Regs* r) {
    const Unknown_00427cd1_Segment* seg = reinterpret_cast<const Unknown_00427cd1_Segment*>(static_cast<uintptr_t>(r->edi));
    const Unknown_00427cd1_Car* car = reinterpret_cast<const Unknown_00427cd1_Car*>(static_cast<uintptr_t>(r->esi));

    int32_t value = seg->WordC8;
    const int32_t bound = (car->Flags71 & 8u) == 0u ? seg->Word54 : seg->Word52;
    if ((car->FlagsFE & 0x40u) != 0u) {
        value = ((value - bound) >> 1) + bound;
    }

    bool viaCallee = (car->Flags9C & 0x10u) != 0u;
    if (!viaCallee && (car->Flags163 & 0x80u) != 0u) {
        if ((car->Flags122 & 0x80u) != 0u) {
            value = bound - (value - bound);
        } else if ((car->Flags163 & 0x40u) != 0u) {
            viaCallee = true;
        }
    }

    if (viaCallee) {
        gp4::Regs call = *r;
        call.st_in = 0;
        call.st_out = 0;
        call.eax = static_cast<uint32_t>(static_cast<int32_t>(car->Word90));
        call.ecx = static_cast<uint32_t>(value);
        gp4::call_regs(0x00427bc1, call);
        r->eax = call.eax;
        r->eflags = (r->eflags & ~0x8d5u) | (call.eflags & 0x8d5u);
        return;
    }

    const uint32_t pitStart = GP4_GLOBAL(uint32_t, 0x00891500);
    uint32_t result = static_cast<uint32_t>(value);
    uint32_t flags = cmpFlags32(r->edi, pitStart);
    if (r->edi < pitStart) {
        const int64_t product = static_cast<int64_t>(static_cast<int32_t>(result - static_cast<uint32_t>(bound)))
            * static_cast<int64_t>(GP4_GLOBAL(int32_t, 0x00624858));
        const uint32_t scaled = static_cast<uint32_t>(static_cast<uint64_t>(product) >> 14);
        result = scaled + static_cast<uint32_t>(bound);
        flags = addFlags32(scaled, static_cast<uint32_t>(bound));
    }

    r->eax = result;
    r->eflags = (r->eflags & ~0x8d5u) | flags;
}
GP4_IMPL(0x00427cd1, FUN_00427cd1, "regs(edx:u32, edi:rec[0x8943a0,0x160,0..29], esi:ptr[0x16b]:bytes) -> (eax, cf, pf, af, zf, sf, of); globals=fuzz:i32+layout=ring_008943a0+words_008943a0+segw_00427bc1+line_008943a0")
