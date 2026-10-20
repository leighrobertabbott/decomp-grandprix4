#include <gp4/gp4.h>

static inline uint32_t parityFlag8(uint32_t value) {
    uint32_t p = value & 0xffu;
    p ^= p >> 4;
    p ^= p >> 2;
    p ^= p >> 1;
    return (p & 1u) == 0u ? 0x04u : 0u;
}

// Flags of the 32-bit `sub left, right` / `cmp left, right` (CF PF AF ZF SF OF).
static inline uint32_t subFlags32(uint32_t left, uint32_t right) {
    const uint32_t result = left - right;
    return (left < right ? 0x01u : 0u)
        | parityFlag8(result)
        | (((left ^ right ^ result) & 0x10u) != 0u ? 0x10u : 0u)
        | (result == 0u ? 0x40u : 0u)
        | ((result & 0x80000000u) != 0u ? 0x80u : 0u)
        | (((left ^ right) & (left ^ result) & 0x80000000u) != 0u ? 0x800u : 0u);
}

// FUNCTION: GP4 0x0043dbdc
// Runs the speed/steering update 0x0043da95 for the car in esi. When the car's word +0x9a is below
// [0x604140], |eax| reaches [0x604144] and byte [0x625a4e] is not 0xff, it first searches a new
// scale for [0x604116] with 0x00405552 (edi is parked in [0x602d84] / [0x602d88]); the scale is
// restored afterwards. The word's change goes to [0x626998]. Every register but edx is restored;
// edx leaves as the original word, and the flags are those of the final `sub eax, edx`.
void __cdecl FUN_0043dbdc(gp4::Regs* r) {
    uint8_t* car = reinterpret_cast<uint8_t*>(static_cast<uintptr_t>(r->esi));
    const uint32_t originalWord = GP4_FIELD(uint16_t, car, 0x9a);
    const uint16_t savedScale = GP4_GLOBAL(uint16_t, 0x00604116);

    gp4::Regs c = *r;
    c.st_in = 0;
    c.st_out = 0;
    c.edx = originalWord;

    if (static_cast<int32_t>(originalWord) < GP4_GLOBAL(int32_t, 0x00604140)) {
        int32_t magnitude = static_cast<int32_t>(r->eax);
        if (magnitude < 0) {
            magnitude = static_cast<int32_t>(0u - static_cast<uint32_t>(magnitude));
        }
        c.ecx = static_cast<uint32_t>(magnitude);
        if (magnitude >= GP4_GLOBAL(int32_t, 0x00604144) && GP4_GLOBAL(uint8_t, 0x00625a4e) != 0xffu) {
            GP4_GLOBAL(uint32_t, 0x00602d84) = r->edi;
            GP4_GLOBAL(uint32_t, 0x00602d88) = r->edi;
            gp4::call_regs(0x00405552, c);
            GP4_GLOBAL(uint16_t, 0x00604116) = static_cast<uint16_t>(c.eax);
        }
    }

    gp4::call_regs(0x0043da95, c);
    GP4_GLOBAL(uint16_t, 0x00604116) = savedScale;

    const uint32_t newWord = GP4_FIELD(uint16_t, car, 0x9a);
    GP4_GLOBAL(uint32_t, 0x00626998) = newWord - originalWord;
    r->edx = originalWord;
    r->eflags = (r->eflags & ~0x8d5u) | subFlags32(newWord, originalWord);
}
GP4_IMPL(0x0043dbdc, FUN_0043dbdc, "regs(eax:i32, edi:rec[0x8943a0,0x160,0..25], esi:rec[0x66fef0,0x41c,0..21]) -> (edx, cf, pf, af, zf, sf, of); globals=fuzz:i32+layout=cars_0066fef0+bank_004279ab")
