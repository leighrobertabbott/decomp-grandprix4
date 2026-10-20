#include <gp4/gp4.h>

static inline uint32_t parityFlag(uint32_t value) {
    value &= 0xffu;
    value ^= value >> 4;
    value ^= value >> 2;
    value ^= value >> 1;
    return (value & 1u) == 0u ? 4u : 0u;
}

// Flags of the 16-bit `left + right`.
static inline uint32_t addFlags16(uint16_t left, uint16_t right) {
    const uint16_t result = static_cast<uint16_t>(left + right);
    return parityFlag(result)
        | (result < left ? 0x01u : 0u)
        | (((left ^ right ^ result) & 0x10u) != 0u ? 0x10u : 0u)
        | (result == 0u ? 0x40u : 0u)
        | ((result & 0x8000u) != 0u ? 0x80u : 0u)
        | ((((left ^ result) & (right ^ result)) & 0x8000u) != 0u ? 0x800u : 0u);
}

struct Unknown_00437fc4_State {
    uint8_t Unknown00[0x90];
    int16_t Unknown90;
};

struct Unknown_00437fc4_Table {
    uint16_t Unknown00;
    uint8_t Unknown02[0x5e];
    uint16_t Unknown60;
    uint16_t Unknown62;
};

// pushal ... popal wrapper: stores angle(0x400, (bx * word[edi+0x60 or 0x62]) >> 5) + word[edi] at
// 0x00629dd2; the sign of word [esi+0x90] picks the factor and negates bx. The flags are those of the
// final 16-bit add (popal does not restore them).
// FUNCTION: GP4 0x00437fc4
void __cdecl FUN_00437fc4(gp4::Regs* r) {
    const Unknown_00437fc4_State* state = reinterpret_cast<const Unknown_00437fc4_State*>(static_cast<uintptr_t>(r->esi));
    const Unknown_00437fc4_Table* table = reinterpret_cast<const Unknown_00437fc4_Table*>(static_cast<uintptr_t>(r->edi));

    int32_t scale = static_cast<int32_t>(static_cast<int16_t>(r->ebx));
    uint32_t factor;
    if (state->Unknown90 < 0) {
        scale = -scale;
        factor = table->Unknown60;
    } else {
        factor = table->Unknown62;
    }
    const int64_t product = static_cast<int64_t>(static_cast<int32_t>(factor)) * static_cast<int64_t>(scale);
    const int32_t scaled = static_cast<int32_t>(static_cast<uint32_t>(product)) >> 5;

    gp4::Regs call = *r;
    call.eax = static_cast<uint32_t>(scaled);
    call.edx = 0x400u;
    gp4::call_regs(0x00401315, call);

    const uint16_t angle = static_cast<uint16_t>(call.eax);
    const uint16_t base = table->Unknown00;
    GP4_GLOBAL(uint16_t, 0x00629dd2) = static_cast<uint16_t>(angle + base);
    r->eflags = (r->eflags & ~0x8d5u) | addFlags16(angle, base);
}
GP4_IMPL(0x00437fc4, FUN_00437fc4, "regs(ebx:u32, esi:ptr[0x98]:bytes, edi:ptr[0x68]:bytes) -> (cf, pf, af, zf, sf, of)")
