#include <gp4/gp4.h>

struct Unknown_004180b0 {
    uint8_t Unknown00[0x9a];
    int16_t Unknown9A;
    uint8_t Unknown9C[0x2ec - 0x9c];
    int32_t Unknown2EC[4];
};

static inline uint32_t parityFlag(uint32_t value) {
    uint32_t parity = value & 0xffu;
    parity ^= parity >> 4;
    parity ^= parity >> 2;
    parity ^= parity >> 1;
    return (parity & 1u) == 0u ? 4u : 0u;
}

// flags of "cmp r32, imm" (left - right)
static inline uint32_t compare32Flags(uint32_t left, uint32_t right) {
    const uint32_t result = left - right;
    return (left < right ? 1u : 0u)
        | parityFlag(result)
        | (((left ^ right ^ result) & 0x10u) != 0u ? 0x10u : 0u)
        | (result == 0u ? 0x40u : 0u)
        | ((result & 0x80000000u) != 0u ? 0x80u : 0u)
        | (((left ^ right) & (left ^ result) & 0x80000000u) != 0u ? 0x800u : 0u);
}

// flags of "cmp r16, imm16"
static inline uint32_t compare16Flags(uint32_t left, uint32_t right) {
    const uint32_t result = (left - right) & 0xffffu;
    return (left < right ? 1u : 0u)
        | parityFlag(result)
        | (((left ^ right ^ result) & 0x10u) != 0u ? 0x10u : 0u)
        | (result == 0u ? 0x40u : 0u)
        | ((result & 0x8000u) != 0u ? 0x80u : 0u)
        | (((left ^ right) & (left ^ result) & 0x8000u) != 0u ? 0x800u : 0u);
}

// Sets DAT_00622485 to 0xff, then back to 0 when the word at +0x9a is below 0x755 and one of
// the four dwords at +0x2ec is below 0x1400. eax ends as the last value compared.
// FUNCTION: GP4 0x004180b0
void __cdecl FUN_004180b0(gp4::Regs* r) {
    const Unknown_004180b0* body = reinterpret_cast<const Unknown_004180b0*>(r->esi);
    GP4_GLOBAL(uint8_t, 0x00622485) = 0xff;

    const int16_t word = body->Unknown9A;
    if (word >= 0x755) {
        r->eax = (r->eax & 0xffff0000u) | (uint32_t)(uint16_t)word;
        r->eflags = (r->eflags & ~0x8d5u) | compare16Flags((uint16_t)word, 0x755u);
        return;
    }

    for (int i = 0; i < 4; ++i) {
        const int32_t value = body->Unknown2EC[i];
        r->eax = (uint32_t)value;
        r->eflags = (r->eflags & ~0x8d5u) | compare32Flags((uint32_t)value, 0x1400u);
        if (value < 0x1400) {
            GP4_GLOBAL(uint8_t, 0x00622485) = 0;
            return;
        }
    }
}
GP4_IMPL(0x004180b0, FUN_004180b0, "regs(eax:u32, esi:ptr[0x300]:bytes) -> (eax, cf, pf, af, zf, sf, of)")
