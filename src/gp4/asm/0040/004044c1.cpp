#include <gp4/gp4.h>

static inline uint32_t parityFlag(uint32_t value) {
    value &= 0xffu;
    value ^= value >> 4;
    value ^= value >> 2;
    value ^= value >> 1;
    return (value & 1u) == 0u ? 4u : 0u;
}

// Flags of the 16-bit `left + right`.
static inline uint32_t addFlags16(uint32_t left, uint32_t right) {
    const uint32_t result = (left + right) & 0xffffu;
    left &= 0xffffu;
    right &= 0xffffu;
    return (result < left ? 1u : 0u)
        | parityFlag(result)
        | ((left ^ right ^ result) & 0x10u)
        | (result == 0u ? 0x40u : 0u)
        | ((result >> 8) & 0x80u)
        | ((~(left ^ right) & (left ^ result) & 0x8000u) != 0u ? 0x800u : 0u);
}

// edi = rotation record (signed word +0x14 is a Q15 factor), esi = point record. Sets the word at
// esi+0x92 to word[esi+0x96] minus (word[edi+0x14] * word[esi+0x90]) >> 15 (16-bit wrap; just the
// word[esi+0x96] when the factor is zero), then runs the rotation 0x004044f2, whose eax/edx/flags it
// returns.
// FUNCTION: GP4 0x004044c1
void __cdecl FUN_004044c1(gp4::Regs* r) {
    uint8_t* rotation = reinterpret_cast<uint8_t*>(static_cast<uintptr_t>(r->edi));
    uint8_t* point = reinterpret_cast<uint8_t*>(static_cast<uintptr_t>(r->esi));

    const int16_t factor = GP4_FIELD(int16_t, rotation, 0x14);
    uint32_t edx = r->edx;
    uint32_t delta = 0u;
    if (factor != 0) {
        const int32_t product = static_cast<int32_t>(factor) * static_cast<int32_t>(GP4_FIELD(int16_t, point, 0x90));
        const uint32_t high = static_cast<uint32_t>(product) >> 15 & 0xffffu;
        edx = (edx & 0xffff0000u) | high;
        delta = 0u - high;
    }
    const uint32_t offset = GP4_FIELD(uint16_t, point, 0x96);
    const uint32_t sum = (delta + offset) & 0xffffu;
    GP4_FIELD(uint16_t, point, 0x92) = static_cast<uint16_t>(sum);

    gp4::Regs c = *r;
    c.st_in = 0;
    c.st_out = 0;
    c.eax = (r->eax & 0xffff0000u) | sum;
    c.edx = edx;
    c.eflags = (r->eflags & ~0x8d5u) | addFlags16(delta, offset);
    gp4::call_regs(0x004044f2, c);

    r->eax = c.eax;
    r->ebx = c.ebx;
    r->ecx = c.ecx;
    r->edx = c.edx;
    r->esi = c.esi;
    r->edi = c.edi;
    r->ebp = c.ebp;
    r->eflags = (r->eflags & ~0x8d5u) | (c.eflags & 0x8d5u);
}
GP4_IMPL(0x004044c1, FUN_004044c1, "regs(edi:ptr[0xd8]:layout=body_0043ebc2, esi:ptr[0xa0]:bytes) -> (eax, edx, cf, pf, af, zf, sf, of)")
