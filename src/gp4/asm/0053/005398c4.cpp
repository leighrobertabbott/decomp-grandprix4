#include <gp4/gp4.h>
#include <gp4/flags.h>

// Compares the value at [ebx] with eax at the width given by the low two bits of the byte +0xd of the
// esi record (0 byte, 1 word, 2 dword) through the jump table at 0x005398e0; 3 (and the unreachable
// 4) runs the empty routine 0x0054e232 inside pushal/popal. edx is saved and restored; the flags are
// those of the `cmp`, or of the `add edx, 0x005398e0` (edx = width * 4) when no compare is done.
// FUNCTION: GP4 0x005398c4
void __cdecl FUN_005398c4(gp4::Regs* r) {
    const uint32_t width = GP4_FIELD(uint8_t, r->esi, 0xd) & 3u;
    uint32_t arith = gp4::flags::add32(width << 2, 0x005398e0u);

    switch (width) {
    case 0:
        arith = gp4::flags::sub8(r->eax, GP4_FIELD(uint8_t, r->ebx, 0));
        break;
    case 1:
        arith = gp4::flags::sub16(r->eax, GP4_FIELD(uint16_t, r->ebx, 0));
        break;
    case 2:
        arith = gp4::flags::sub32(r->eax, GP4_FIELD(uint32_t, r->ebx, 0));
        break;
    default:
        gp4::cdecl_<0x0054e232, void>();
        break;
    }
    r->eflags = gp4::flags::merge(r->eflags, arith);
}
GP4_IMPL(0x005398c4, FUN_005398c4, "regs(eax:u32, ebx:ptr[8]:bytes, esi:ptr[0x40]:bytes) -> (cf, pf, af, zf, sf, of)")
