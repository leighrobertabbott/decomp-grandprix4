#include <gp4/gp4.h>

static inline uint32_t parityFlag(uint32_t value) {
    value &= 0xffu;
    value ^= value >> 4;
    value ^= value >> 2;
    value ^= value >> 1;
    return (value & 1u) == 0u ? 4u : 0u;
}

// Flags of a 32-bit logic result (and): CF = OF = AF = 0.
static inline uint32_t logicFlags(uint32_t result) {
    return parityFlag(result) | (result == 0u ? 0x40u : 0u) | ((result & 0x80000000u) != 0u ? 0x80u : 0u);
}

// Arithmetic flags (CF/PF/AF/ZF/SF/OF) of `add left, right`.
static inline uint32_t addFlags(uint32_t left, uint32_t right) {
    const uint32_t result = left + right;
    return (result < left ? 1u : 0u) | parityFlag(result) | ((left ^ right ^ result) & 0x10u)
        | (result == 0u ? 0x40u : 0u) | ((result >> 24) & 0x80u)
        | ((~(left ^ right) & (left ^ result) & 0x80000000u) >> 20);
}

struct Unknown_0040da62_Segment {
    uint8_t Unknown00[0x44];
    uint32_t Link44;
    uint8_t Unknown48[0x7a - 0x48];
    uint16_t Index7A;
};

// Address of the 8-byte entry (read unaligned) of the table at 0x00604435 for a segment record.
static inline uint32_t entryAddress(const Unknown_0040da62_Segment* segment) {
    return 0x00604435u + static_cast<uint32_t>(segment->Index7A) * 8u;
}

// Fallback result: ecx is recomputed from the segment (imul ecx, ecx, 8 ; add ecx, 0x604435, whose
// flags are the result), edx keeps its high word and takes the entry's first word, eax keeps its
// high word and takes the word at esi+0x66.
static inline void fallback(gp4::Regs* r, uint8_t* source, const Unknown_0040da62_Segment* segment,
                            uint32_t eaxBase, uint32_t edxBase, uint32_t ebx) {
    const uint32_t entry = entryAddress(segment);
    r->ecx = entry;
    r->edx = (edxBase & 0xffff0000u) | GP4_FIELD(uint16_t, entry, 0);
    r->eax = (eaxBase & 0xffff0000u) | GP4_FIELD(uint16_t, source, 0x66);
    r->ebx = ebx;
    r->edi = static_cast<uint32_t>(reinterpret_cast<uintptr_t>(segment));
    r->eflags = (r->eflags & ~0x8d5u) | addFlags(static_cast<uint32_t>(segment->Index7A) * 8u, 0x00604435u);
}

// Interpolates between the table values (unaligned entry word +4 shifted left by 14 plus the signed
// word +6) of the segment record at edi and of the next record (wrapping to 0x008943a0 at the lap
// end [0x00866ecc]) by the signed word [esi+0x66]: edx = whole part (>> 14), eax = fraction
// (& 0x3fff). When the link at +0x44 is zero or either entry's word +4 is zero, eax/edx keep their
// high words and take [esi+0x66] and the entry's first word instead.
// FUNCTION: GP4 0x0040da62
void __cdecl FUN_0040da62(gp4::Regs* r) {
    uint8_t* source = reinterpret_cast<uint8_t*>(static_cast<uintptr_t>(r->esi));
    const Unknown_0040da62_Segment* segment =
        reinterpret_cast<const Unknown_0040da62_Segment*>(static_cast<uintptr_t>(r->edi));

    if (segment->Link44 == 0u) {
        fallback(r, source, segment, r->eax, r->edx, r->ebx);
        return;
    }
    const uint32_t firstEntry = entryAddress(segment);
    const uint32_t firstWord = GP4_FIELD(uint16_t, firstEntry, 4);
    if (firstWord == 0u) {
        fallback(r, source, segment, 0u, r->edx, r->ebx);
        return;
    }
    const int32_t firstOffset = static_cast<int32_t>(GP4_FIELD(int16_t, firstEntry, 6));
    const uint32_t start = (firstWord << 14) + static_cast<uint32_t>(firstOffset);

    uint32_t next = static_cast<uint32_t>(reinterpret_cast<uintptr_t>(segment)) + 0x160u;
    if (!(next < GP4_GLOBAL(uint32_t, 0x00866ecc))) {
        next = 0x008943a0u;
    }
    segment = reinterpret_cast<const Unknown_0040da62_Segment*>(static_cast<uintptr_t>(next));

    const uint32_t secondEntry = entryAddress(segment);
    const uint32_t secondWord = GP4_FIELD(uint16_t, secondEntry, 4);
    if (secondWord == 0u) {
        fallback(r, source, segment, 0u, static_cast<uint32_t>(firstOffset), start);
        return;
    }
    const int32_t secondOffset = static_cast<int32_t>(GP4_FIELD(int16_t, secondEntry, 6));
    const uint32_t end = (secondWord << 14) + static_cast<uint32_t>(secondOffset);

    const int32_t fraction = static_cast<int32_t>(GP4_FIELD(int16_t, source, 0x66));
    const int64_t product = static_cast<int64_t>(static_cast<int32_t>(end - start)) * static_cast<int64_t>(fraction);
    const uint32_t interpolated = static_cast<uint32_t>(static_cast<uint64_t>(product) >> 14) + start;

    r->ebx = start;
    r->ecx = secondEntry;
    r->ebp = static_cast<uint32_t>(fraction);
    r->edi = next;
    r->edx = interpolated >> 14;
    r->eax = interpolated & 0x3fffu;
    r->eflags = (r->eflags & ~0x8d5u) | logicFlags(r->eax);
}
GP4_IMPL(0x0040da62, FUN_0040da62, "regs(esi:ptr[0x70]:bytes, edi:rec[0x8943a0,0x160,0..24], eax:u32, ebx:u32, edx:u32, ebp:u32) -> (eax, ebx, ecx, edx, ebp, edi, cf, pf, af, zf, sf, of); globals=layout=lanes_0040da62")
