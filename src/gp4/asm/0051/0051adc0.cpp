#include <gp4/gp4.h>
#include <gp4/flags.h>

// esi points at a car record: its +0x1c is the current track-segment pointer (edi), whose word
// at +0x1a carries an 11-bit value; edx = [edi + 0x1a] & 0x7ff, flags of that `and`.
// FUNCTION: GP4 0x0051adc0
void __cdecl FUN_0051adc0(gp4::Regs* r) {
    const uint32_t edi = GP4_FIELD(uint32_t, r->esi, 0x1c);
    const uint32_t edx = GP4_FIELD(uint32_t, edi, 0x1a) & 0x7ffu;
    r->edi = edi;
    r->edx = edx;
    r->eflags = gp4::flags::merge(r->eflags, gp4::flags::logic32(edx));
}
GP4_IMPL(0x0051adc0, FUN_0051adc0, "regs(esi:ptr[0x20]:layout=car_0051adc0) -> (edx, edi, cf, pf, af, zf, sf, of)")
