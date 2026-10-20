#include <gp4/gp4.h>
#include <gp4/flags.h>

// eax indexes the pointer table at 0x006f2f6c (a track-segment pointer); the word at +0x1a of the
// segment carries an 11-bit value: eax = [seg + 0x1a] & 0x7ff, edi = seg, flags of the `and`.
// FUNCTION: GP4 0x0051adcd
void __cdecl FUN_0051adcd(gp4::Regs* r) {
    const uint32_t index = r->eax;
    const uint32_t seg = GP4_ARRAY(uint32_t, 0x006f2f6c)[index];
    const uint32_t value = GP4_FIELD(uint32_t, seg, 0x1a) & 0x7ffu;
    r->edi = seg;
    r->eax = value;
    r->eflags = gp4::flags::merge(r->eflags, gp4::flags::logic32(value));
}
GP4_IMPL(0x0051adcd, FUN_0051adcd, "regs(eax:u32[0..15]) -> (eax, edi, cf, pf, af, zf, sf, of); globals=layout=ptrs_006f2f6c")
