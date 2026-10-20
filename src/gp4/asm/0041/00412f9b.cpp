#include <gp4/gp4.h>

static inline uint32_t parityFlag8(uint32_t value) {
    uint32_t p = value & 0xffu;
    p ^= p >> 4;
    p ^= p >> 2;
    p ^= p >> 1;
    return (p & 1u) == 0u ? 0x04u : 0u;
}

// Flags of a 16-bit logic result (xor / test): CF = OF = AF = 0.
static inline uint32_t logicFlags16(uint32_t result) {
    result &= 0xffffu;
    return parityFlag8(result)
        | (result == 0u ? 0x40u : 0u)
        | ((result & 0x8000u) != 0u ? 0x80u : 0u);
}

// Flags of the 16-bit `left - right`.
static inline uint32_t subFlags16(uint32_t left, uint32_t right) {
    left &= 0xffffu;
    right &= 0xffffu;
    const uint32_t result = (left - right) & 0xffffu;
    return (left < right ? 0x01u : 0u)
        | parityFlag8(result)
        | (((left ^ right ^ result) & 0x10u) != 0u ? 0x10u : 0u)
        | (result == 0u ? 0x40u : 0u)
        | ((result & 0x8000u) != 0u ? 0x80u : 0u)
        | ((((left ^ right) & (left ^ result)) & 0x8000u) != 0u ? 0x800u : 0u);
}

// Flags of the 16-bit `neg value`.
static inline uint32_t negFlags16(uint32_t value) {
    value &= 0xffffu;
    const uint32_t result = (0u - value) & 0xffffu;
    return (value != 0u ? 0x01u : 0u)
        | parityFlag8(result)
        | (((value ^ result) & 0x10u) != 0u ? 0x10u : 0u)
        | (result == 0u ? 0x40u : 0u)
        | ((result & 0x8000u) != 0u ? 0x80u : 0u)
        | (value == 0x8000u ? 0x800u : 0u);
}

// edi = the track-segment record (stored to the pair 0x00602d84 / 0x00602d88 the drivetrain reads),
// esi = the engine record. 0x004058cf returns a 32-bit sample (eax; its low word goes to 0x00639644).
// Both products are kept as Q14 words: D = bits 14..29 of word[0x0063964c] * word[0x00639648] (stored
// to 0x00934df0) and A = bits 14..29 of low16((sample * [0x00638d20]) >> 14) * word[0x00603934]. When D
// and A have different signs the result is A; otherwise it is |A| - |D| (0 when negative), with the
// sign of A, stored to word 0x00603454 and left in ax. dx holds D (or |D|), bx the last magnitude
// difference, and the flags are those of the last xor / sub / test / neg.
// FUNCTION: GP4 0x00412f9b
void __cdecl FUN_00412f9b(gp4::Regs* r) {
    GP4_GLOBAL(uint32_t, 0x00602d88) = r->edi;
    GP4_GLOBAL(uint32_t, 0x00602d84) = r->edi;

    gp4::Regs s = *r;
    s.st_in = 0;
    s.st_out = 0;
    gp4::call_regs(0x004058cf, s);

    const uint32_t sample = s.eax;
    GP4_GLOBAL(uint16_t, 0x00639644) = static_cast<uint16_t>(sample);

    // imul word [0x639648] (ax = word [0x63964c]); two shl / rcl pairs keep dx:ax bits 14..29 in dx.
    const int32_t first = static_cast<int32_t>(static_cast<int16_t>(GP4_GLOBAL(uint16_t, 0x0063964c)))
        * static_cast<int32_t>(static_cast<int16_t>(GP4_GLOBAL(uint16_t, 0x00639648)));
    const uint16_t reference = static_cast<uint16_t>(static_cast<uint32_t>(first) >> 14);
    GP4_GLOBAL(uint16_t, 0x00934df0) = reference;

    // imul dword [0x638d20]; shrd eax, edx, 14.
    const int64_t scaled = static_cast<int64_t>(static_cast<int32_t>(sample))
        * static_cast<int64_t>(GP4_GLOBAL(int32_t, 0x00638d20));
    uint32_t eax = static_cast<uint32_t>(static_cast<uint64_t>(scaled) >> 14);
    uint32_t edx = static_cast<uint32_t>(static_cast<uint64_t>(scaled) >> 32);

    // imul word [0x603934] on ax, again keeping bits 14..29.
    const int32_t second = static_cast<int32_t>(static_cast<int16_t>(eax))
        * static_cast<int32_t>(static_cast<int16_t>(GP4_GLOBAL(uint16_t, 0x00603934)));
    const uint16_t value = static_cast<uint16_t>(static_cast<uint32_t>(second) >> 14);

    uint32_t ebx = s.ebx;
    uint16_t result = value;
    uint32_t flags;
    const uint16_t mixed = static_cast<uint16_t>(reference ^ value);
    flags = logicFlags16(mixed);
    if ((mixed & 0x8000u) != 0u) {
        edx = (edx & 0xffff0000u) | mixed;
    } else {
        uint16_t magnitudeD = reference;
        if ((magnitudeD & 0x8000u) != 0u) {
            magnitudeD = static_cast<uint16_t>(0u - magnitudeD);
        }
        uint16_t magnitudeA = value;
        if ((magnitudeA & 0x8000u) != 0u) {
            magnitudeA = static_cast<uint16_t>(0u - magnitudeA);
        }
        edx = (edx & 0xffff0000u) | magnitudeD;
        uint16_t difference = static_cast<uint16_t>(magnitudeA - magnitudeD);
        flags = subFlags16(magnitudeA, magnitudeD);
        ebx = (ebx & 0xffff0000u) | difference;
        if ((difference & 0x8000u) != 0u) {
            result = 0u;
        } else {
            flags = logicFlags16(value);
            if ((value & 0x8000u) != 0u) {
                flags = negFlags16(difference);
                difference = static_cast<uint16_t>(0u - difference);
                ebx = (ebx & 0xffff0000u) | difference;
            }
            result = difference;
        }
    }
    GP4_GLOBAL(uint16_t, 0x00603454) = result;
    eax = (eax & 0xffff0000u) | result;

    r->eax = eax;
    r->ebx = ebx;
    r->ecx = s.ecx;
    r->edx = edx;
    r->esi = s.esi;
    r->edi = s.edi;
    r->ebp = s.ebp;
    r->eflags = (r->eflags & ~0x8d5u) | (flags & 0x8d5u);
}
GP4_IMPL(0x00412f9b, FUN_00412f9b, "regs(ebx:u32, esi:ptr[0x242]:bytes, edi:rec[0x8943a0,0x160,0..29]) -> (eax, ebx, ecx, edx, cf, pf, af, zf, sf, of); globals=fuzz:i32+layout=ring_008943a0+words_00413090+engine_00413606")
