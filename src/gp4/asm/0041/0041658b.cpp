#include <gp4/gp4.h>

static inline uint32_t parityFlag(uint32_t value) {
    value &= 0xffu;
    value ^= value >> 4;
    value ^= value >> 2;
    value ^= value >> 1;
    return (value & 1u) == 0u ? 4u : 0u;
}

static inline uint32_t additionFlags32(uint32_t left, uint32_t right) {
    const uint32_t result = left + right;
    return (result < left ? 1u : 0u)
        | parityFlag(result)
        | (((left ^ right ^ result) & 0x10u) != 0u ? 0x10u : 0u)
        | (result == 0u ? 0x40u : 0u)
        | ((result & 0x80000000u) != 0u ? 0x80u : 0u)
        | ((~(left ^ right) & (left ^ result) & 0x80000000u) != 0u ? 0x800u : 0u);
}

// FUNCTION: GP4 0x0041658b
// Stores into the word at DAT_00639642 the difference (high word of the dword at 0x638030, sign extended)
// minus an angle: that same high word when |word +0x9a of the record in esi| < 0x3ab (difference 0), else
// the angle FUN_00401240 returns for the dwords at 0x63804c / 0x638054. Leaves eax = the difference,
// ebp = the high word, edx = 0x638054 in the second case (else unchanged), flags of the final "add eax, ebp".
void __cdecl FUN_0041658b(gp4::Regs* r) {
    uint8_t* body = reinterpret_cast<uint8_t*>(static_cast<uintptr_t>(r->esi));
    const uint32_t reference = static_cast<uint32_t>(GP4_GLOBAL(int32_t, 0x00638030) >> 16);

    int32_t magnitude = GP4_FIELD(int16_t, body, 0x9a);
    if (magnitude < 0) {
        magnitude = static_cast<int16_t>(0u - static_cast<uint32_t>(magnitude));
    }

    uint32_t angle = reference;
    uint32_t edx = r->edx;
    if (magnitude >= 0x3ab) {
        gp4::Regs call = *r;
        call.st_in = 0;
        call.st_out = 0;
        call.eax = GP4_GLOBAL(uint32_t, 0x0063804c);
        call.edx = GP4_GLOBAL(uint32_t, 0x00638054);
        gp4::call_regs(0x00401240, call);
        angle = call.eax;
        edx = call.edx;
    }

    const uint32_t negated = 0u - angle;
    const uint32_t difference = negated + reference;
    GP4_GLOBAL(uint16_t, 0x00639642) = static_cast<uint16_t>(difference);

    r->eax = difference;
    r->ebp = reference;
    r->edx = edx;
    r->eflags = (r->eflags & ~0x8d5u) | additionFlags32(negated, reference);
}
GP4_IMPL(0x0041658b, FUN_0041658b, "regs(edx:u32, esi:ptr[0xa4]:bytes) -> (eax, edx, ebp, cf, pf, af, zf, sf, of); globals=fuzz:i32")
