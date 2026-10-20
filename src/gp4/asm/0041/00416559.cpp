#include <gp4/gp4.h>

static inline uint32_t parityFlag(uint32_t value) {
    value &= 0xffu;
    value ^= value >> 4;
    value ^= value >> 2;
    value ^= value >> 1;
    return (value & 1u) == 0u ? 4u : 0u;
}

// Flags of "cmp word left, right" (left - right on 16 bits).
static inline uint32_t compareFlags16(uint32_t left, uint32_t right) {
    const uint32_t result = (left - right) & 0xffffu;
    return (left < right ? 1u : 0u)
        | parityFlag(result)
        | (((left ^ right ^ result) & 0x10u) != 0u ? 0x10u : 0u)
        | (result == 0u ? 0x40u : 0u)
        | ((result & 0x8000u) != 0u ? 0x80u : 0u)
        | ((((left ^ right) & (left ^ result)) & 0x8000u) != 0u ? 0x800u : 0u);
}

// FUNCTION: GP4 0x00416559
// Writes the word at +0x18 of the record in esi: the word at +0x0e when |word +0x9a| < 0x3ab, else the
// angle FUN_00401240 returns for the dwords (x = +0xa4, y = +0xa8). In the first case eax keeps its high
// half and the flags are those of "cmp ax, 0x3ab"; in the second eax/edx/flags come from the callee.
void __cdecl FUN_00416559(gp4::Regs* r) {
    uint8_t* body = reinterpret_cast<uint8_t*>(static_cast<uintptr_t>(r->esi));

    uint32_t magnitude = GP4_FIELD(uint16_t, body, 0x9a);
    if ((magnitude & 0x8000u) != 0u) {
        magnitude = (0u - magnitude) & 0xffffu;
    }

    if (static_cast<int16_t>(magnitude) < 0x3ab) {
        const uint16_t value = GP4_FIELD(uint16_t, body, 0x0e);
        GP4_FIELD(uint16_t, body, 0x18) = value;
        r->eax = (r->eax & 0xffff0000u) | value;
        r->eflags = (r->eflags & ~0x8d5u) | compareFlags16(magnitude, 0x3abu);
        return;
    }

    gp4::Regs angle = *r;
    angle.st_in = 0;
    angle.st_out = 0;
    angle.eax = GP4_FIELD(uint32_t, body, 0xa4);
    angle.edx = GP4_FIELD(uint32_t, body, 0xa8);
    gp4::call_regs(0x00401240, angle);

    GP4_FIELD(uint16_t, body, 0x18) = static_cast<uint16_t>(angle.eax);
    r->eax = angle.eax;
    r->edx = angle.edx;
    r->eflags = (r->eflags & ~0x8d5u) | (angle.eflags & 0x8d5u);
}
GP4_IMPL(0x00416559, FUN_00416559, "regs(eax:u32, edx:u32, esi:ptr[0xb0]:bytes) -> (eax, edx, cf, pf, af, zf, sf, of)")
