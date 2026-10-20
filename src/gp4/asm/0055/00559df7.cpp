#include <gp4/gp4.h>
#include <gp4/flags.h>

// Fills four derived words of the record at edi from its inputs:
//   +0x100 = (u16[+0x60] + 0x50) * u8[+0x28] * 8      +0x104 = i32[+0xec] - 0x40000000
//   +0x108 = 0                                        +0x10a = u16[+0xbc] << 2
// Register effects: ecx = the zero-extended byte, edx = high half of the (always non-negative,
// < 2^31) imul product = 0, eax = (+0x104 value & 0xffff0000) | (shifted word). The flags are those
// of the final `shl ax, 2` (CF = bit 14, OF = bit14 ^ bit13, AF = 0).
// FUNCTION: GP4 0x00559df7
void __cdecl FUN_00559df7(gp4::Regs* r) {
    uint8_t* rec = (uint8_t*)r->edi;

    const uint32_t mult = GP4_FIELD(uint8_t, rec, 0x28);
    const uint32_t base = (uint32_t)GP4_FIELD(uint16_t, rec, 0x60) + 0x50u;
    const uint64_t prod = (uint64_t)base * (uint64_t)mult;
    GP4_FIELD(uint32_t, rec, 0x100) = (uint32_t)prod << 3;

    const uint32_t diff = GP4_FIELD(uint32_t, rec, 0xec) - 0x40000000u;
    GP4_FIELD(uint32_t, rec, 0x104) = diff;
    GP4_FIELD(uint16_t, rec, 0x108) = 0;

    const uint16_t word = GP4_FIELD(uint16_t, rec, 0xbc);
    const uint16_t shifted = (uint16_t)(word << 2);
    GP4_FIELD(uint16_t, rec, 0x10a) = shifted;

    const uint32_t flags = gp4::flags::szp(shifted, 16)
                         | (((uint32_t)word >> 14) & 1u ? gp4::flags::CF : 0u)
                         | ((((uint32_t)word >> 14) ^ ((uint32_t)word >> 13)) & 1u ? gp4::flags::OF : 0u);

    r->eax = (diff & 0xffff0000u) | shifted;
    r->ecx = mult;
    r->edx = (uint32_t)(prod >> 32);
    r->eflags = gp4::flags::merge(r->eflags, flags);
}
GP4_IMPL(0x00559df7, FUN_00559df7, "regs(edi:ptr[0x114]:bytes) -> (eax, ecx, edx, cf, pf, af, zf, sf, of)")
