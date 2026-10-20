#include <gp4/gp4.h>

static inline uint32_t parityFlag(uint32_t value) {
    value &= 0xffu;
    value ^= value >> 4;
    value ^= value >> 2;
    value ^= value >> 1;
    return (value & 1u) == 0u ? 4u : 0u;
}

// 16-bit `left - right`: flags CF/PF/AF/ZF/SF/OF.
static inline uint32_t subFlags16(uint32_t left, uint32_t right, uint32_t result) {
    return (left < right ? 1u : 0u) | parityFlag(result) | ((left ^ right ^ result) & 0x10u)
        | ((result & 0xffffu) == 0u ? 0x40u : 0u) | ((result >> 8) & 0x80u)
        | ((((left ^ right) & (left ^ result)) & 0x8000u) >> 4);
}

// 16-bit `left + right`: flags CF/PF/AF/ZF/SF/OF.
static inline uint32_t addFlags16(uint32_t left, uint32_t right, uint32_t result) {
    return ((left + right) > 0xffffu ? 1u : 0u) | parityFlag(result) | ((left ^ right ^ result) & 0x10u)
        | ((result & 0xffffu) == 0u ? 0x40u : 0u) | ((result >> 8) & 0x80u)
        | (((~(left ^ right) & (left ^ result)) & 0x8000u) >> 4);
}

// ax = word[ebx+0x68] - word[esi+0x68], plus the word at 0x00868558 when negative; the word is
// stored to 0x00603afa and 0x0043b51c runs with ax = that value and the flags of the last
// arithmetic instruction.
// FUNCTION: GP4 0x0043b559
void __cdecl FUN_0043b559(gp4::Regs* r) {
    uint8_t* lhs = reinterpret_cast<uint8_t*>(static_cast<uintptr_t>(r->ebx));
    uint8_t* rhs = reinterpret_cast<uint8_t*>(static_cast<uintptr_t>(r->esi));
    const uint32_t a = GP4_FIELD(uint16_t, lhs, 0x68);
    const uint32_t b = GP4_FIELD(uint16_t, rhs, 0x68);

    uint32_t result = (a - b) & 0xffffu;
    uint32_t flags = subFlags16(a, b, result);
    if ((result & 0x8000u) != 0u) {
        const uint32_t addend = GP4_GLOBAL(uint16_t, 0x00868558);
        const uint32_t sum = (result + addend) & 0xffffu;
        flags = addFlags16(result, addend, sum);
        result = sum;
    }
    GP4_GLOBAL(uint16_t, 0x00603afa) = static_cast<uint16_t>(result);

    r->eax = (r->eax & 0xffff0000u) | result;
    r->eflags = (r->eflags & ~0x8d5u) | flags;
    gp4::call_regs(0x0043b51c, *r);
}
GP4_IMPL(0x0043b559, FUN_0043b559, "regs(eax:u32, ebx:rec[0x66fef0,0x41c,0..21], esi:rec[0x66fef0,0x41c,0..21], ecx:u32, edx:u32) -> (eax, ecx, edx, edi, cf, pf, af, zf, sf, of); globals=layout=cars_0066fef0")
