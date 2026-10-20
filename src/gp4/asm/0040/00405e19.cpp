#include <gp4/gp4.h>

static inline uint32_t parityFlag(uint32_t value) {
    value &= 0xffu;
    value ^= value >> 4;
    value ^= value >> 2;
    value ^= value >> 1;
    return (value & 1u) == 0u ? 4u : 0u;
}

// Flags of `left + right` whose result is `result`.
static inline uint32_t addFlags(uint32_t left, uint32_t right, uint32_t result) {
    return (result < left ? 1u : 0u) | parityFlag(result) | ((left ^ right ^ result) & 0x10u)
        | (result == 0u ? 0x40u : 0u) | ((result >> 24) & 0x80u)
        | ((((left ^ result) & (right ^ result)) & 0x80000000u) >> 20);
}

struct Unknown_00405e19 {
    uint8_t Unknown00[4];
    int16_t Unknown04;
    uint8_t Unknown06[2];
    int16_t Unknown08;
    uint8_t Unknown0A[2];
    int16_t Unknown0C;
    int16_t Unknown0E;
};

// Dot product of the (ecx, ebx) offset from the point (word +4, word +8) of the record at edi with
// the direction (word +0xe, word +0xc) >> 6. eax = low 32 bits of the sum, edx = high half of the
// last 64-bit product, CF = sign of the sum (stc / clc); ebx and ecx are saved and restored.
// FUNCTION: GP4 0x00405e19
void __cdecl FUN_00405e19(gp4::Regs* r) {
    const Unknown_00405e19* rec = reinterpret_cast<const Unknown_00405e19*>(static_cast<uintptr_t>(r->edi));

    const int32_t dx = static_cast<int32_t>(r->ecx - static_cast<uint32_t>(static_cast<int32_t>(rec->Unknown04)));
    const int32_t dz = static_cast<int32_t>(r->ebx - static_cast<uint32_t>(static_cast<int32_t>(rec->Unknown08)));

    const int32_t dirZ = static_cast<int32_t>(rec->Unknown0C) >> 6;
    const int64_t productZ = static_cast<int64_t>(dirZ) * static_cast<int64_t>(dz);
    const uint32_t lowZ = static_cast<uint32_t>(productZ);

    const int32_t dirX = static_cast<int32_t>(rec->Unknown0E) >> 6;
    const int64_t productX = static_cast<int64_t>(dirX) * static_cast<int64_t>(dx);
    const uint32_t lowX = static_cast<uint32_t>(productX);
    const uint32_t highX = static_cast<uint32_t>(static_cast<uint64_t>(productX) >> 32);

    const uint32_t sum = lowX + lowZ;
    uint32_t flags = addFlags(lowX, lowZ, sum);
    flags = (flags & ~1u) | ((sum & 0x80000000u) != 0u ? 1u : 0u);

    r->eax = sum;
    r->edx = highX;
    r->eflags = (r->eflags & ~0x8d5u) | flags;
}
GP4_IMPL(0x00405e19, FUN_00405e19, "regs(ebx:u32, ecx:u32, edi:ptr[0x18]:bytes) -> (eax, edx, cf, pf, af, zf, sf, of)")
