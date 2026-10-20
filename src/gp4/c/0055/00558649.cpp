#include <gp4/gp4.h>
#include <gp4/flags.h>

// Stream handler (mode byte 0x0086857c == 1, else only esi += 4): reads two words at esi (esi += 4).
// The first replaces the word at 0x00934dd8; the second, sign-extended and shifted left by 12, is stored
// in 0x00934ddc and left in eax. The flags are those of `shl eax, 0xc` (other modes: the `add esi, 4`).
// FUNCTION: GP4 0x00558649
void __cdecl FUN_00558649(gp4::Regs* r) {
    const uint32_t source = r->esi;
    r->esi = source + 4u;
    if (GP4_GLOBAL(uint8_t, 0x0086857c) != 1u) {
        r->eflags = gp4::flags::merge(r->eflags, gp4::flags::add32(source, 4u));
        return;
    }

    GP4_GLOBAL(uint16_t, 0x00934dd8) = GP4_FIELD(uint16_t, source, 0);
    const uint32_t value = static_cast<uint32_t>(static_cast<int32_t>(static_cast<int16_t>(GP4_FIELD(uint16_t, source, 2))));
    const uint32_t result = value << 12;
    GP4_GLOBAL(uint32_t, 0x00934ddc) = result;

    // shl by 12: CF = last bit shifted out, AF = 0, OF = bit31(value << 11 ^ result).
    uint32_t arith = gp4::flags::szp(result, 32);
    arith |= (value >> 20) & 1u;
    arith |= ((((value << 11) ^ result) >> 31) & 1u) << 11;
    r->eax = result;
    r->eflags = gp4::flags::merge(r->eflags, arith);
}
GP4_IMPL(0x00558649, FUN_00558649, "regs(eax:u32, esi:ptr[0x10]:bytes) -> (eax, esi, cf, pf, af, zf, sf, of); globals=fuzz:i32+layout=mode_0086857c")
