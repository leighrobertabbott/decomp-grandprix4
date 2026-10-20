#include <gp4/gp4.h>
#include <gp4/flags.h>

// Stores eax at [ebx] at the width given by the low two bits of the byte +0xd of the esi record
// (0 byte, 1 word, 2 dword) through the jump table at 0x00539a30; 3 (and the unreachable fifth
// entry) runs the empty routine 0x0054e232 inside pushal/popal. edx is saved and restored; the flags
// are those of the `add edx, 0x00539a30` (edx = width * 4).
// FUNCTION: GP4 0x00539a13
void __cdecl FUN_00539a13(gp4::Regs* r) {
    const uint32_t width = GP4_FIELD(uint8_t, r->esi, 0xd) & 3u;
    const uint32_t arith = gp4::flags::add32(width << 2, 0x00539a30u);

    switch (width) {
    case 0:
        GP4_FIELD(uint8_t, r->ebx, 0) = static_cast<uint8_t>(r->eax);
        break;
    case 1:
        GP4_FIELD(uint16_t, r->ebx, 0) = static_cast<uint16_t>(r->eax);
        break;
    case 2:
        GP4_FIELD(uint32_t, r->ebx, 0) = r->eax;
        break;
    default:
        gp4::cdecl_<0x0054e232, void>();
        break;
    }
    r->eflags = gp4::flags::merge(r->eflags, arith);
}
GP4_IMPL(0x00539a13, FUN_00539a13, "regs(eax:u32, ebx:ptr[8]:bytes, esi:ptr[0x40]:bytes) -> (cf, pf, af, zf, sf, of)")
