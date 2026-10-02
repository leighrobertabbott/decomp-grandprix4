#include <gp4/gp4.h>

struct Unknown_004030c6_Source {
    uint8_t Unknown00[0x18];
    int16_t Unknown18;
    uint8_t Unknown1A[0x15e];
    int16_t Unknown178;
};

struct Unknown_004030c6_Record {
    uint8_t Unknown00[0x66];
    int16_t Unknown66;
    uint8_t Unknown68[0x150];
    Unknown_004030c6_Source* Unknown1B8;
    uint8_t Unknown1BC[0x25a];
    int16_t Unknown416;
};

static inline uint32_t parityFlag8(uint32_t value) {
    uint32_t parity = value & 0xffu;
    parity ^= parity >> 4;
    parity ^= parity >> 2;
    parity ^= parity >> 1;
    return (parity & 1u) == 0u ? 4u : 0u;
}

static inline uint32_t additionFlags32(uint32_t left, uint32_t right) {
    const uint32_t result = left + right;
    return (result < left ? 1u : 0u)
        | parityFlag8(result)
        | (((left ^ right ^ result) & 0x10u) != 0u ? 0x10u : 0u)
        | (result == 0u ? 0x40u : 0u)
        | ((result & 0x80000000u) != 0u ? 0x80u : 0u)
        | ((~(left ^ right) & (left ^ result) & 0x80000000u) != 0u ? 0x800u : 0u);
}

// FUNCTION: GP4 0x004030c6
void __cdecl FUN_004030c6(gp4::Regs* r) {
    Unknown_004030c6_Record* record = reinterpret_cast<Unknown_004030c6_Record*>(r->esi);
    const int16_t* pair = reinterpret_cast<const int16_t*>(r->edi);
    const Unknown_004030c6_Source* source = record->Unknown1B8;

    const int16_t startWord = static_cast<int16_t>(static_cast<uint16_t>(source->Unknown18) + static_cast<uint16_t>(pair[0]));
    const int32_t start = startWord;
    const int16_t endWord = static_cast<int16_t>(static_cast<uint16_t>(source->Unknown178) + static_cast<uint16_t>(pair[0xb0]));
    const int16_t spanWord = static_cast<int16_t>(static_cast<uint16_t>(endWord) - static_cast<uint16_t>(startWord));
    const int32_t span = spanWord;
    const int32_t factor = record->Unknown66;

    const int64_t product = static_cast<int64_t>(span) * static_cast<int64_t>(factor);
    const uint32_t scaled = static_cast<uint32_t>(static_cast<uint64_t>(product) >> 14);
    const uint32_t sum = scaled + static_cast<uint32_t>(start);

    record->Unknown416 = static_cast<int16_t>(sum);

    r->eax = sum;
    r->ecx = static_cast<uint32_t>(start);
    r->edx = static_cast<uint32_t>(static_cast<uint64_t>(product) >> 32);
    r->eflags = (r->eflags & ~0x8d5u) | additionFlags32(scaled, static_cast<uint32_t>(start));
}
GP4_IMPL(0x004030c6, FUN_004030c6, "regs(edi:ptr[0x168]:bytes, esi:ptr[0x41e]:layout=rec_004030c6) -> (eax, ecx, edx, cf, pf, af, zf, sf, of)")
