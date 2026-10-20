#include <gp4/gp4.h>

static inline uint32_t parityFlag(uint32_t value) {
    value &= 0xffu;
    value ^= value >> 4;
    value ^= value >> 2;
    value ^= value >> 1;
    return (value & 1u) == 0u ? 4u : 0u;
}

struct Unknown_00402fcd_Edi {
    int16_t Unknown00;
    int16_t Unknown02;
    uint8_t Unknown04[0x162 - 0x04];
    int16_t Unknown162;
};

struct Unknown_00402fcd_Esi {
    uint8_t Unknown00[0x18];
    uint16_t Unknown18;
    uint8_t Unknown1A[0x66 - 0x1a];
    int16_t Unknown66;
    uint8_t Unknown68[0x9a - 0x68];
    int16_t Unknown9A;
};

// Entry of the signed 16-bit table at 0x007c5c5c for a 16-bit angle-like value:
// ((value >> 2) & 0xfffe) is the byte offset.
static inline int32_t tableAt(uint16_t value) {
    const uint32_t offset = (static_cast<uint32_t>(value) >> 2) & 0xfffeu;
    return GP4_ARRAY(int16_t, 0x007c5c5c)[offset >> 1];
}

// Interpolates between the words at +2 and +0x162 of the edi record by the factor in word +0x66 of
// the esi record, projects it through the table at 0x007c5c5c with the distance of word +0x18 from
// word +0 of the edi record, folds the result back (0x4000 - ...) into a second table lookup and
// scales that by word +0x9a. eax = low half of the product << 2, edx = its high half; the flags are
// those of the final `shl eax, 2`. ecx is preserved.
// FUNCTION: GP4 0x00402fcd
void __cdecl FUN_00402fcd(gp4::Regs* r) {
    const Unknown_00402fcd_Edi* edi = reinterpret_cast<const Unknown_00402fcd_Edi*>(static_cast<uintptr_t>(r->edi));
    const Unknown_00402fcd_Esi* esi = reinterpret_cast<const Unknown_00402fcd_Esi*>(static_cast<uintptr_t>(r->esi));

    const int32_t start = edi->Unknown02;
    const int32_t span = static_cast<int32_t>(edi->Unknown162) - start;
    const int64_t blend = static_cast<int64_t>(span) * static_cast<int32_t>(esi->Unknown66);
    const int32_t position = start + static_cast<int32_t>(static_cast<uint32_t>(static_cast<uint64_t>(blend) >> 14));

    uint16_t angle = static_cast<uint16_t>(esi->Unknown18 - static_cast<uint16_t>(edi->Unknown00));
    if (static_cast<int16_t>(angle) < 0) {
        angle = static_cast<uint16_t>(0u - angle);
    }
    const int64_t projected = static_cast<int64_t>(tableAt(angle)) * position;
    const uint16_t low = static_cast<uint16_t>(static_cast<uint32_t>(static_cast<uint64_t>(projected) >> 14));

    uint16_t folded = static_cast<uint16_t>(static_cast<uint16_t>(0u - low) + 0x4000u);
    if (static_cast<int16_t>(folded) < 0) {
        folded = static_cast<uint16_t>(0u - folded);
    }
    const int64_t product = static_cast<int64_t>(tableAt(folded)) * static_cast<int32_t>(esi->Unknown9A);

    const uint32_t before = static_cast<uint32_t>(product);
    const uint32_t result = before << 2;
    // shl by 2 (pinned oracle model): CF = last bit out, AF = 0, OF = bit31(value << 1 ^ result)
    const uint32_t flags = ((before >> 30) & 1u)
        | parityFlag(result)
        | (result == 0u ? 0x40u : 0u)
        | ((result >> 24) & 0x80u)
        | ((((before << 1) ^ result) >> 20) & 0x800u);

    r->eax = result;
    r->edx = static_cast<uint32_t>(static_cast<uint64_t>(product) >> 32);
    r->eflags = (r->eflags & ~0x8d5u) | flags;
}
GP4_IMPL(0x00402fcd, FUN_00402fcd, "regs(edi:ptr[0x16c]:bytes, esi:ptr[0xa2]:bytes) -> (eax, edx, cf, pf, af, zf, sf, of)")
