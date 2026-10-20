#include <gp4/gp4.h>

static inline uint32_t parityFlag(uint32_t value) {
    value &= 0xffu;
    value ^= value >> 4;
    value ^= value >> 2;
    value ^= value >> 1;
    return (value & 1u) == 0u ? 4u : 0u;
}

struct Unknown_004048da_Subject {
    uint8_t Unknown00[0x18];
    uint16_t Angle18;
    uint8_t Unknown1A[0x71 - 0x1a];
    uint8_t Flags71;
    uint8_t Unknown72[0x98 - 0x72];
    int32_t Scale98;
    uint8_t Unknown9C[0xa4 - 0x9c];
    int32_t ComponentA4;
    int32_t ComponentA8;
    uint8_t UnknownAC[0xe5 - 0xac];
    uint8_t FlagsE5;
};

// Q14 table value (int16 word at 0x007c5c5c) of a 16-bit angle, folded to its absolute value.
static inline int32_t tableValue(uint32_t angle) {
    uint32_t folded = angle & 0xffffu;
    if ((folded & 0x8000u) != 0u) {
        folded = (0u - folded) & 0xffffu;
    }
    const uint32_t offset = (folded >> 2) & 0xfffeu;
    return *reinterpret_cast<const int16_t*>(GP4_ARRAY(const uint8_t, 0x007c5c5c) + offset);
}

// Splits the scale at +0x98 into two components: +0xa4 = table(0x4000 - angle) * scale >> 14 and
// +0xa8 = table(angle) * scale >> 14, where angle is the word at +0x18, turned half a circle
// (+0x8000, temporarily) when bit 0 of +0xe5 is clear and bit 2 of +0x71 is set. The angle word is
// restored; ax of eax is restored from the angle (eax is pushed after "mov ax, [esi+0x18]"), edx is
// the high dword of the last product and the flags are those of the last "shrd eax, edx, 14"
// (pinned oracle model: AF = 0, CF = bit 13 of the product, OF = bit31(product>>13 ^ product>>14)).
// FUNCTION: GP4 0x004048da
void __cdecl FUN_004048da(gp4::Regs* r) {
    Unknown_004048da_Subject* subject =
        reinterpret_cast<Unknown_004048da_Subject*>(static_cast<uintptr_t>(r->esi));
    const uint32_t savedEax = (r->eax & 0xffff0000u) | subject->Angle18;

    uint32_t angle = subject->Angle18;
    if ((subject->FlagsE5 & 1u) == 0u && (subject->Flags71 & 4u) != 0u) {
        angle = (angle + 0x8000u) & 0xffffu;
    }
    const int32_t scale = subject->Scale98;

    const int64_t first = static_cast<int64_t>(tableValue(0x4000u - angle)) * static_cast<int64_t>(scale);
    subject->ComponentA4 = static_cast<int32_t>(static_cast<uint32_t>(static_cast<uint64_t>(first) >> 14));

    const uint64_t second = static_cast<uint64_t>(static_cast<int64_t>(tableValue(angle)) * static_cast<int64_t>(scale));
    const uint32_t previous = static_cast<uint32_t>(second >> 13);
    const uint32_t result = static_cast<uint32_t>(second >> 14);
    subject->ComponentA8 = static_cast<int32_t>(result);

    const uint32_t flags = (previous & 1u)
        | parityFlag(result)
        | (result == 0u ? 0x40u : 0u)
        | ((result & 0x80000000u) != 0u ? 0x80u : 0u)
        | (((previous ^ result) & 0x80000000u) != 0u ? 0x800u : 0u);

    r->eax = savedEax;
    r->edx = static_cast<uint32_t>(second >> 32);
    r->eflags = (r->eflags & ~0x8d5u) | flags;
}
GP4_IMPL(0x004048da, FUN_004048da, "regs(eax:u32, esi:ptr[0xed]:bytes) -> (eax, edx, cf, pf, af, zf, sf, of)")
