#include <gp4/gp4.h>
#include <gp4/flags.h>

// FUNCTION: GP4 0x0052a919
// Draws three colour components of the 6-byte record at edi (bytes +5, +4, +3; each scaled by
// 100/100, i.e. unchanged) with colour words 0x2110, 0x3198 and 0x421f stored through 0x531af4
// (DAT_00827559) and, for every nonzero component, a call to the empty stub 0x531b2e. The
// running position ecx starts at [0x81d1cc] + 0x74 and moves back by each component. The
// whole routine is a pushal/popal wrapper (ebx/ebp/ecx/eax are scratch), so the only visible
// effects are the stored colour word and the flags of the last flag writer: `or eax, eax` of
// the byte at +3 when it is zero, else the `add eax, ecx` after the `neg`.
void __cdecl FUN_0052a919(gp4::Regs* r) {
    const uint8_t* rec = reinterpret_cast<const uint8_t*>(r->edi);
    const uint32_t blue = rec[3];
    const uint32_t green = rec[4];
    const uint32_t red = rec[5];
    uint32_t pos = GP4_GLOBAL(uint32_t, 0x0081d1cc) + 0x74u;

    gp4::Regs c = *r;
    c.eax = 0x2110u;
    gp4::call_regs(0x00531af4, c);
    if (red != 0u) {
        pos = pos - red;
        gp4::cdecl_<0x00531b2e, void>();
    }

    c.eax = 0x3198u;
    gp4::call_regs(0x00531af4, c);
    if (green != 0u) {
        pos = pos - green;
        gp4::cdecl_<0x00531b2e, void>();
    }

    c.eax = 0x421fu;
    gp4::call_regs(0x00531af4, c);
    uint32_t flags = gp4::flags::logic32(0u);
    if (blue != 0u) {
        flags = gp4::flags::add32(0u - blue, pos);
        gp4::cdecl_<0x00531b2e, void>();
    }
    r->eflags = gp4::flags::merge(r->eflags, flags);
}
GP4_IMPL(0x0052a919, FUN_0052a919, "regs(edi:ptr[0x40]:u8{0,0,0,1,0x40,0x7f,0xff}) -> (cf, pf, af, zf, sf, of); globals=fuzz:i32")
