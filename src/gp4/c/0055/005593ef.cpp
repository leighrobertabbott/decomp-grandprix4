#include <gp4/gp4.h>
#include <gp4/flags.h>

// Reads the flag byte +0x2c of the record at [0x891500] into al; when bit 7 is set the mask-update
// routine 0x00559391 runs on that record with dl = 0x80, then, when bit 6 of the byte read at the
// start is set, with dl = 0x40 (al is not reloaded). edi is pushed and popped. eax is left with al =
// the byte read; edx and the flags are those the last callee leaves (edx kept, flags of the test
// when no call ran).
// FUNCTION: GP4 0x005593ef
void __cdecl FUN_005593ef(gp4::Regs* r) {
    const uint32_t record = GP4_GLOBAL(uint32_t, 0x00891500);
    const uint32_t al = GP4_FIELD(uint8_t, record, 0x2c);
    gp4::Regs c = *r;
    c.edi = record;
    c.eax = (r->eax & 0xffffff00u) | al;
    uint32_t flags = gp4::flags::logic8(al & 0x80u);
    if ((al & 0x80u) != 0u) {
        c.edx = (c.edx & 0xffffff00u) | 0x80u;
        gp4::call_regs(0x00559391, c);
        flags = c.eflags;
    }
    flags = gp4::flags::logic8(al & 0x40u);
    if ((al & 0x40u) != 0u) {
        c.edx = (c.edx & 0xffffff00u) | 0x40u;
        gp4::call_regs(0x00559391, c);
        flags = c.eflags;
    }
    r->eax = c.eax;
    r->edx = c.edx;
    r->eflags = gp4::flags::merge(r->eflags, flags);
}
GP4_IMPL(0x005593ef, FUN_005593ef, "regs(eax:u32, edx:u32) -> (eax, edx, cf, pf, af, zf, sf, of); globals=layout=ring_008943a0+bits_0055616f+flags2c_0055774b")
