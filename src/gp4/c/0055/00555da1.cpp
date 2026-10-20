#include <gp4/gp4.h>
#include <gp4/flags.h>

// Advances esi by 4 past a two-word entry {divisor, value}. When byte [0x0086857c] == 1 it also derives a step: a zero
// divisor stores the value in [0x00874dd8] and clears [0x00874de0] / [0x00874de8]; otherwise it stores the divisor in
// [0x00874de0] and (value - [0x00874dd8]) / divisor in [0x00874de8]. The body is a pushal/popal pair, so only esi and
// the flags of the final `add esi, 4` change.
// FUNCTION: GP4 0x00555da1
void __cdecl FUN_00555da1(gp4::Regs* r) {
    const uint32_t esi = r->esi;
    if (GP4_GLOBAL(uint8_t, 0x0086857c) == 1u) {
        const uint16_t* entry = reinterpret_cast<const uint16_t*>(static_cast<uintptr_t>(esi));
        const uint32_t divisor = entry[0];
        if (divisor == 0u) {
            GP4_GLOBAL(uint32_t, 0x00874dd8) = entry[1];
            GP4_GLOBAL(uint32_t, 0x00874de0) = 0u;
            GP4_GLOBAL(uint32_t, 0x00874de8) = 0u;
        } else {
            GP4_GLOBAL(uint32_t, 0x00874de0) = divisor;
            const int32_t delta = static_cast<int32_t>(static_cast<uint32_t>(entry[1]) - GP4_GLOBAL(uint32_t, 0x00874dd8));
            GP4_GLOBAL(uint32_t, 0x00874de8) = static_cast<uint32_t>(delta / static_cast<int32_t>(GP4_GLOBAL(uint32_t, 0x00874de0)));
        }
    }
    r->esi = esi + 4u;
    r->eflags = gp4::flags::merge(r->eflags, gp4::flags::add32(esi, 4u));
}
GP4_IMPL(0x00555da1, FUN_00555da1, "regs(esi:ptr[8]:u16{0,0,1,2,3,0x10,0x100,0x7fff,0x8000,0xffff}) -> (esi, cf, pf, af, zf, sf, of); globals=fuzz:i32")
