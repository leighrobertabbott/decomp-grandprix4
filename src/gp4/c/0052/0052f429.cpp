#include <gp4/gp4.h>
#include <gp4/flags.h>

// Counts the set bits among the lowest n bits of the mask at 0x0081e5d4, where n = table[al] - 0x61 (an
// 8-bit value; the character in al is translated by 0x0054d755 first, i.e. the position of a letter
// 'a'.. in the alphabet). eax = the count (the whole register is rewritten); ebx and ecx are saved and
// restored. The flags are those of `sub al, 0x61` when n is 0 (jecxz), else those of the final `dec ecx`
// (ZF and PF set, CF kept from the last `shr ebx, 1`).
// FUNCTION: GP4 0x0052f429
void __cdecl FUN_0052f429(gp4::Regs* r) {
    gp4::Regs c = *r;
    gp4::call_regs(0x0054d755, c);

    const uint32_t letter = c.eax & 0xffu;
    uint32_t arith = gp4::flags::sub8(letter, 0x61u);
    uint32_t steps = (letter - 0x61u) & 0xffu;
    uint32_t count = 0u;
    if (steps != 0u) {
        uint32_t mask = GP4_GLOBAL(uint32_t, 0x0081e5d4);
        uint32_t carry;
        do {
            carry = mask & 1u;
            mask >>= 1;
            if (carry != 0u) {
                count += 1u;
            }
            steps -= 1u;
        } while (steps != 0u);
        arith = gp4::flags::dec32(1u, carry);                   // the last `dec ecx` keeps the shifted-out CF
    }
    r->eax = count;
    r->eflags = gp4::flags::merge(r->eflags, arith);
}
GP4_IMPL(0x0052f429, FUN_0052f429, "regs(eax:u32) -> (eax, cf, pf, af, zf, sf, of); globals=fuzz:i32+layout=letters_0052f429")
