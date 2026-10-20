#include <gp4/gp4.h>
#include <gp4/flags.h>

// Stream handler (mode byte 0x0086857c == 1, else only esi += 2): reads one word w at esi (esi += 2),
// stores w << 16 in 0x00874b60 and sets the byte 0x00874b65 to 0xff. eax ends as w << 16 (the word
// replaces ax, then `shl eax, 0x10`); the flags are those of that shift (other modes: the `add esi, 2`).
// FUNCTION: GP4 0x00558197
void __cdecl FUN_00558197(gp4::Regs* r) {
    const uint32_t source = r->esi;
    r->esi = source + 2u;
    if (GP4_GLOBAL(uint8_t, 0x0086857c) != 1u) {
        r->eflags = gp4::flags::merge(r->eflags, gp4::flags::add32(source, 2u));
        return;
    }

    const uint32_t value = (r->eax & 0xffff0000u) | GP4_FIELD(uint16_t, source, 0);
    const uint32_t result = value << 16;
    GP4_GLOBAL(uint32_t, 0x00874b60) = result;
    GP4_GLOBAL(uint8_t, 0x00874b65) = 0xffu;

    // shl by 16: CF = last bit shifted out, AF = 0, OF = bit31(value << 15 ^ result).
    uint32_t arith = gp4::flags::szp(result, 32);
    arith |= (value >> 16) & 1u;
    arith |= ((((value << 15) ^ result) >> 31) & 1u) << 11;
    r->eax = result;
    r->eflags = gp4::flags::merge(r->eflags, arith);
}
GP4_IMPL(0x00558197, FUN_00558197, "regs(eax:u32, esi:ptr[0x10]:bytes) -> (eax, esi, cf, pf, af, zf, sf, of); globals=fuzz:i32+layout=mode_0086857c")
