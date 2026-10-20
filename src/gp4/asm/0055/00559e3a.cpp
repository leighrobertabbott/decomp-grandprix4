#include <gp4/gp4.h>
#include <gp4/flags.h>

// Twin of 0x00559df7 for the second half of the record: fills four derived words from its inputs:
//   +0x10c = (u16[+0x62] + 0x50) * u8[+0x29] * 8      +0x110 = i32[+0xec] + 0x40000000
//   +0x114 = 0                                        +0x116 = u16[+0xbe] << 2
// Register effects: ecx = the zero-extended byte, edx = high half of the (always non-negative,
// < 2^31) imul product = 0, eax = (+0x110 value & 0xffff0000) | (shifted word). The flags are those
// of the final `shl ax, 2` (CF = bit 14, OF = bit14 ^ bit13, AF = 0).
// FUNCTION: GP4 0x00559e3a
void __cdecl FUN_00559e3a(gp4::Regs* r) {
    uint8_t* rec = (uint8_t*)r->edi;

    const uint32_t mult = GP4_FIELD(uint8_t, rec, 0x29);
    const uint32_t base = (uint32_t)GP4_FIELD(uint16_t, rec, 0x62) + 0x50u;
    const uint64_t prod = (uint64_t)base * (uint64_t)mult;
    GP4_FIELD(uint32_t, rec, 0x10c) = (uint32_t)prod << 3;

    const uint32_t sum = GP4_FIELD(uint32_t, rec, 0xec) + 0x40000000u;
    GP4_FIELD(uint32_t, rec, 0x110) = sum;
    GP4_FIELD(uint16_t, rec, 0x114) = 0;

    const uint16_t word = GP4_FIELD(uint16_t, rec, 0xbe);
    const uint16_t shifted = (uint16_t)(word << 2);
    GP4_FIELD(uint16_t, rec, 0x116) = shifted;

    const uint32_t flags = gp4::flags::szp(shifted, 16)
                         | (((uint32_t)word >> 14) & 1u ? gp4::flags::CF : 0u)
                         | ((((uint32_t)word >> 14) ^ ((uint32_t)word >> 13)) & 1u ? gp4::flags::OF : 0u);

    r->eax = (sum & 0xffff0000u) | shifted;
    r->ecx = mult;
    r->edx = (uint32_t)(prod >> 32);
    r->eflags = gp4::flags::merge(r->eflags, flags);
}
GP4_IMPL(0x00559e3a, FUN_00559e3a, "regs(edi:ptr[0x120]:bytes) -> (eax, ecx, edx, cf, pf, af, zf, sf, of)")
