#include <gp4/gp4.h>
#include <gp4/flags.h>

// The selector is computed in a noinline helper so the compiler keeps the (unreachable) clamp and
// the fifth table entry that the original has.
static __declspec(noinline) uint32_t SelectorOf(uint32_t record) {
    return GP4_FIELD(uint8_t, record, 0xd) & 3u;
}

// Loads an unsigned value from [ebx] into eax at the width given by the low two bits of the byte +0xd
// of the esi record (0 byte, 1 word, 2 dword, 3 zero) through the jump table at 0x00539960; the
// unreachable fifth entry runs the empty routine 0x0054e232 inside pushal/popal. edx is saved and
// restored; the flags are those of the `add edx, 0x00539960` (edx = width * 4).
// FUNCTION: GP4 0x00539944
void __cdecl FUN_00539944(gp4::Regs* r) {
    uint32_t width = SelectorOf(r->esi);
    if (width >= 5u) {
        width = 4u;
    }
    const uint32_t arith = gp4::flags::add32(width << 2, 0x00539960u);

    switch (width) {
    case 0:
        r->eax = GP4_FIELD(uint8_t, r->ebx, 0);
        break;
    case 1:
        r->eax = GP4_FIELD(uint16_t, r->ebx, 0);
        break;
    case 2:
        r->eax = GP4_FIELD(uint32_t, r->ebx, 0);
        break;
    case 3:
        r->eax = 0u;
        break;
    default:
        gp4::cdecl_<0x0054e232, void>();
        break;
    }
    r->eflags = gp4::flags::merge(r->eflags, arith);
}
GP4_IMPL(0x00539944, FUN_00539944, "regs(ebx:ptr[8]:bytes, esi:ptr[0x40]:bytes) -> (eax, cf, pf, af, zf, sf, of)")
